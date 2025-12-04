### MONAD - Modular Object/NTuple Analysis Dataflow

A lightweight header-only library inspired by [FairROOT](https://github.com/FairRootGroup/FairRoot) and 
[Go4](https://github.com/gsi-ee/go4) that allows for modular and functional analysis embedded in
CERN ROOT framework.

- Supports writing and reading of single objects (Object-Wise De/Serialization - OWS) directly into/from a ROOT file.
  - Almost all data types supported, even mixing TObject/std:: -like types
  - Nested arrays/structures supported
- **Type driven**
  - All (de)serialization target type compatibility checked at compilation time.
  - Strict compiler checks.
- Multithreading 
  - Split entry pool into chunks, process via *worker pools*
  - Can opt into pure single-threaded for *e.g.* cluster tasks.
- Columnar data stored in [RNTuple](https://root.cern/doc/master/group__tutorial__ntuple.html) format

Dependencies are C++17, ROOT 6.34+ and [boost](https://www.boost.org/doc/user-guide/getting-started.html).

#### Installation
Grab a hold of the project via `git clone` with the only `monad.hxx` file to be included. 
The `Makefile` in this project is just an example.

It is also possible to only grab the header file.

#### Short Description and minimal example.
The main two abstracted ingredients are called *containers* and *processors*.
We represent and encapsulate physics data together with a name tag into various *containers*.
A *processor* then analyses data from input containers and stores it into a single unique output container.

A *process* is a sequence of processors which represents one *step* of the analysis. Any output container of
a previous step, becomes input container for the next step.

Complete *analysis* is a sequence of steps that convert raw binary data into fully analysed data.

Each step must output an intermediate ROOT file.
Users are expected to write their own container and processor types which inherit from corresponding 
base MONAD types, define a few functions and then MONAD handles the rest.

##### Containers
A container type encapsulates simultaneously both columnar data (recorded *per entry*) and one or more single-objects (called SO's) 
that are written/read only once - such as parameters or histograms.
To interact with ROOT, each container instance is marked with a unique name tag.

**[1]** Define how you represent the columnar data for this specific analysis sequence/detector. Analogy to branches in a `TTree`.
Each such type must implement `void Clean() noexcept` method and a compiled dictionary.

A hypothetical example - suppose we wish to analyse data from some scintillators:
In the file `sci.h` :
```cpp
#include "monad/monad.hxx"

struct RNSci {
    struct Measurement {
        double x = NAN;
        double t = NAN;
        Measurement() = default;
        Measurement(double x, double t) : x(x), t(t) {}
        
        virtual ~Measurement() = default;
        ClassDef(Measurement, 1);
    };

    double E = NAN;
    std::vector<Measurement> hits;

    inline void Clean() noexcept { E = 0; hits.clear(); }
    virtual ~RNSciCal() = default;
    ClassDef(RNSciCal, 1);
};

/* This is our parameter structure, it represents data to be 
 * written only once, not per entry. */
struct SCIParam { 
    /* ... */
    virtual ~SCIParam() = default; 
    ClassDef(SCIParam, 1);
};

/* Structure representing per-entry data for this detector system. */
struct RNFRS {
    constexpr static i32 N_VALID_SCI = 4;
    std::array<RNSci, N_VALID_SCI> sci;

    inline void Clean() noexcept { // <--- must have this method 
        for(auto& s : sci) s.Clean();
    }
    virtual ~RNFRSCal() = default;
    ClassDef(RNFRSCal, 1);
}
```

**[2]** Wrap your type that will represent a column in `RNTuple` 
into the generic MONAD `TContainer`, and then optionally declare SO's via pointer handles:
```cpp
struct TFRSCont : TContainer<RNFRS> {
    TFRSCont(); // <-- must have some ctor to give it a name.
    void Setup() override; // <-- must have 
    void Init(TDictInfo info) override; // <-- optional

    /* Declare series of SO's here, via pointer handles. 
     * Can be left blank if only columnar data needs to be represented. */
    std::array<SCIParam, RNFRS::N_VALID_SCI> *sci_param{};
    std::string* parameterFile;

    TH1I* h1_x_sci_before_target;
    TH1I* h1_x_sci_after_target;
}
```
**[3]** In the file: `sci.cxx`, define the constructors or some other method which names the container,
plus optionally the inherited initialization method, for example.

```cpp
TFRSCont::TFRSCont() : TContainer("FRS") {}
void TFRSCont::Init(TDictInfo info) { /* ... */ }
```

**[4]** Implement the `void Setup()` method, where you define the SO's. Note that 
the multithreaded collector (more about it later) needs to know how to *sum up* or *average out* two instances
of a SO-type. For `TH1*` types, it is already defined by the `TH1::Add(const TH1&)` method.

```cpp
using T = std::array<SCIParam, RNFRS::N_VALID_SCI>;
void Add(T&, const T&) {} // no-op

void TFRSCont::Setup() {
    /* Arguments in RegisterObject<T> are the same for T's constructor, if the intial arg is const char*, else
     * it has a const char* label as the first argument. */
	h1_x_sci_before_target = RegisterObject<TH1I>("h1_x_sci_before_target", "Position (from Scintillator) before target (mm)", 400,-100,100);
	h1_x_sci_after_target = RegisterObject<TH1I>("h1_x_sci_after_target", "Position (from Scintillator) after target (mm)", 400,-100,100);

	/* Collector deduced from the free Add function above. */
	sci_param = RegisterObject<T>("sci_parameters", {});

    /* Collector function pointer can given as second argument as well. */
	parameterFile = RegisterObject<std::string>("param_file", mnd::noop_fn<std::string>(), "This is the actual payload of the string!");
}
```

**[5]** Implement ROOT dictionary for types that will be serialized.

```cpp
ClassImp(RNSci);
ClassImp(RNSci::Measurement);
ClassImp(RNFRS);
ClassImp(SCIParam);
```

##### Processors
Processor is a structure (type) which holds instances of one or more different 
input and a *single* output container which it owns. Input containers can be of different types, and 
the processor constructor will take a copy of each input container. Due to copy semantics of
`std::shared_ptr<T>` and `T*`, the input data is deserialized only once, and their underlying buffers
are allocated and filled only once per event.

For the same hypothetical example above, we would do the following:

**[1]** Define your own processor type by wrapping together the
MONAD base procesor, and one output and one or more input types. Declare
the two constructors and a `void ProcessEntry() noexcept` method.
This method will be the entry point to the data mapping, and will be invoked *per event*. 

In file `sciproc.h`:
```cpp
#include "sci.h"

/* For some type `TMapCont` which is the input in this case.. */
struct TFRSProc : TProcessor < 
	TFRSCont   // <-- output container type
	(TMapCont) // <-- list of user input container types; one sample type given in this example
> {
    /* Type alias - not necessary but it simplifies syntax. */
    using Base = TProcessor<TFRSCont(TMapCont)>;

    TFRSProc(TFRSCont& out, const TMapCont& in) :
    TFRSProc::Base(out, in) { /* whatever extra code. */ }

    TFRSCalProc() = default;
    void ProcessEntry() noexcept;

    /* Whatever helper methods/fields/statics you wish .. */
    void ProcessSci(int );
};
```

**[2]** Implement the methods/constructors in the implementation file.
Note that even though containers' `void Clean()` class method must be defined, it isn't
called by default. You can call it manually here.
In file `sciproc.cxx`:

```cpp
void TFRSProc::ProcessEntry() noexcept {
    for(int i=0; i < N_VALID_SCI; ++i)
        ProcessSci(i);
}

void TFRSProc::ProcessSci(int n) {
    RNSCI& sci_obj = this->out.sci[n];
    sci_obj.Clean();
    /* ... */ 
}
```

##### Main program
Each step of the analysis is given by a *standalone* program.

```cpp
#include "sciproc.h"

int main(int argc, char** argv) {
```
**[1]** Declare your containers, and pass whatever initialization you wish.
Call their `Setup()` method. `PROG_PATH` is just an example of a preproc directive exported
from `common.mk`.

```
TMapCont mfrs;
mfrs.Setup();

TFRSCont cfrs;
cfrs.Init( {{"Setup", PROG_PATH "/parameters/setup.json"}} );
cfrs.Setup();
```
**[2]** Construct the full analysis process object by chaining the monadic `emplace_process`
method. Users are expected to provide input file name, output file name and name
of the output `RNTuple`.

```
auto pool = TAnalysisProcess<>(inFile, outFile, "rn_example") // <-- 'bare skeleton' process
    .emplace_process<TFRSProc>(/*TFRSProc ctor:*/ cfrs, mfrs) // <-- now holds just one subprocess
    .MakePool<8>( 4096 ) // <-- finalize the process, split into 8 subthreads, each will be handled data in chunks of 4096 entries.
```

**[3]** Optionally define a progress bar to see the execution progression. Check `ProgressBar` API in [indicators](https://github.com/p-ranav/indicators/).
Note, if the `indicators/indicators.hh` file is not present, then skip this step.

```
ProgressBar bar {
    option::BarWidth{50},
    option::Start{"["},
    option::Fill{"="},
    option::Lead{">"},
    option::Remainder{" "},
    option::End{"]"},
    option::PostfixText{"Test"},
    option::ForegroundColor{Color::yellow},
    option::ShowPercentage{true},
    option::ShowElapsedTime{true},
    option::ShowRemainingTime{true},
    option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}
};
```

**[4]** Start the analysis step, `maxEvents` can be specified to limit the total entry number. If left defaulted, it will handle
all entries in the input root file. Otherwise, if the `indicators/indicators.hh` file is not present, then `maxEvents` becomes
the first and only (optional) argument. 
```
pool.Start(bar, maxEvents);	
```

**[5]** `pool`'s destructor will automatically write the output file.
```
} // end of main.
```

##### Detailed descriptions of types
This section is dedicated to much more detailed description of
backbone types, in the jargon of functional programming with a bullet list of useful calls.

###### `TOnce<T>`
Single-object (SO) of type `T` that will go through OWS are wrapped in a `TOnce<T>` type,
together with an `std::string` label. We expose identical API to both `TObject`-derived
types and `std::` -like. The constructor of these types is always the underlying `T`'s ctor, with 
an extra `const char*` initial argument, if type `T` doesn't already handle `(const char*, T&&...)` ctor.
`TOnce<T>` expands the abstract `TOnceBase` to enable polymorphic read and write calls.
If in a multithreaded program, the code needs to know how to *stitch* all of the same SO's (from different threads) back together. 
This problem can be reduced to knowing how to *add* only two instances at a time.
For types such as `TH1`-derived, the `TH1::Add(const TH1&)` method already handles this. 
Generally, compiler will try to find in-order: `T::Add(const T&)`, free `void Add(T&, const T&)` function defined and use that, if users define it.
Same function signatures but with `Mean` symbol come next in priority.
For types where the compiler cannot deduce the collector function, a function pointer `void (*)(T&, const T&)`
can also be given to handle this. If the function pointer is given, it will take priority over compile-time candidates.
Example of types where users either need to define the free fnc or give a fnc ptr is `std::array<T,N>`. 
The final `TAnalysisPool<...>::Collect()` will
propagate the corresponding call and effectively perform a binary fold of all the instances of `TOnce<T>` with the same name (that were given to different threads).

- `T& operator() ();` returns underlying `T&` handle.
- `T* operator-> ();` same but for `T*` ; call `T`'s methods via arrow operator.

###### `TContainer<T>`
Wrapper around `std::shared_ptr<T>` together with a vector of SO handles.
It also holds an `std::string` label, which shall be the name of the corresponding column in the `RNTuple`.
The string label of each of the held `TOnce<T>` objects is prefixed with the container label and an underscore.
It exposes a mixin-style API `U* RegisterObject(const char* name, Ts&& args...)`
which users call in the derived type to instantiate their corresponding SO's pointer handles of types `U*`.

`TContainer<T>` is still an abstract type which will be the parent to user defined containers. Children must override the `void Setup()`
method. In the type definition, for performance reasons, do the cache-alignment by adding `alignas(mnd::CL)` after the struct/class keyword.

- `template<typename U, typename... Ts> U* RegisterObject(const char* name, Ts&&... args);`
- `void Clean();` calls underlying `T`'s `Clean()` method.
- `T& operator*();` return underlying `T&` handle.
- `T& inner();`     return underlying `T&` handle.
- `T* operator->();` same but for `T*` ; call `T`'s methods via arrow operator.

###### `TRawContainer<T>`
Initial step of the analysis usually requires reading from `TTree` rather than `RNTuple`.
This type is used to encapsulate a deserialization target (a `TBranch`) which cling RTTI resolves to type `T`.
Note that SO deserialization isn't supported in this case.

- `T& operator*();` return underlying `T&` handle.
- `T* raw();` return underlying `T*` handle.
- `T* operator->();` same; call `T`'s methods via arrow operator.

###### `TProcessor<Out(Ins...)>`
Backbone type used as a parent to the user-defined processor type, where `Out` is an output container type
and `Ins...` is a list of input container types.
Main constructor is: `explicit TProcessor(Out& , const Ins&...)` which takes ownership of the `Out` instance.
It is purposefully not marked as rvalue ref (for cleaner API), but once the TProcessor has been handed the `Out&`, it owns it.

Derived types will need to define a default ctor, as well as some `(Out&, const Ins...&, /* ... */)` ctor which delegates to this base class
ctor. Main purpose of this derived processor type is define and implement a `void ProcessEntry() noexcept` method which performs the actual analysis.

- `template<uint32_t N=0> decltype(auto) GetInput();` returns reference to N-th input container.
- `.out` - access the output container reference.

###### `TAnalysisProcess<Ts...>`
Wraps around an `std::tuple<Ts...>` of various `Ts = TProcessor<Out(Ins...)>` instances, together with some IO info, thread object and spsc queue. 
The monadic (chainable) `emplace_process` method constructs an extended type of `TAnalysisProcess` with the new process stitched to the back of the tuple.

At the end, a call to `MakePool` will exit this class and hand us back the main `TAnalysisPool` instance.

- `template<typename... Args> auto emplace_process(Args&&... args) && -> TAnalysisProcess<Ts..., U>;`
- `template<uint32_t N> auto MakePool(u32 NSlice) && -> TAnalysisPool<N, Ts...>;`

###### `TAnalysisPool<N, Ts...>`
Final type in the composition/inheritance that holds everything together.
It owns `N > 0` instances of the underlying `TAnalysisProcess<Ts...>` which will be given off one each to its 
underlying threads. 
Setting `N == 1` will not fork to a single worker, but instead propagate the analysis only in the main thread (will
not make `pthread_create()` or similar syscalls). API remains invariant between those two modes.
Condition that `N` is a power of 2 is checked.

- `void SendOneBatch(uint32_t n)` send a batch of entries `n` for some initial analysis. This call will not fill
the output trees (but *can* fill the SO's).
- `void Start()` start the analysis; split the input entry list into batches of size `NSlice` and fans them 
out to workers via round-robin, populating their respective spsc queue with 'job requests'.
