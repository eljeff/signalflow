# SignalFlow

![ci](https://github.com/ideoforms/signal/workflows/ci/badge.svg)

SignalFlow is an audio synthesis framework designed for clear and concise expression of complex musical ideas. Its core is written in C++, with a complete Python interface, and so can be used for audio experimentation in iPython/Jupyter or embedded into cross-platform applications.

SignalFlow is in alpha status. All interfaces may be subject to change.

## Example

```python
from signalflow import *

#--------------------------------------------------------------------------------
# An AudioGraph is made up of a network of interconnected Nodes, which generate
# and process audio. 
#--------------------------------------------------------------------------------
graph = AudioGraph(start=True)

#--------------------------------------------------------------------------------
# Passing an array of frequencies creates a stereo output.
#--------------------------------------------------------------------------------
sine = SineOscillator([440, 880])

#--------------------------------------------------------------------------------
# Simple attack/sustain/release envelope with linear curves.
#--------------------------------------------------------------------------------
env = EnvelopeASR(0.01, 0.1, 0.5)

#--------------------------------------------------------------------------------
# Use standard arithmetic operations to combine signals. When a multi-channel 
# signal is multiplied by a mono signal, the mono signal is auto-upmixed.
#--------------------------------------------------------------------------------
output = sine * env

#--------------------------------------------------------------------------------
# Connect the output to the graph, and begin playback.
#--------------------------------------------------------------------------------
graph.play(output)
graph.wait()
```

To do the same in C++:

```cpp
#include <signalflow/signalflow.h>
using namespace signalflow;

/*------------------------------------------------------------------------
 * Classes ending in *Ref are subclasses of std::shared_ptr which
 * automatically handle memory management.
 *-----------------------------------------------------------------------*/
AudioGraphRef graph = new AudioGraph();

NodeRef sine = new SineOscillator({ 440, 880 });
NodeRef env = new EnvelopeASR(0.01, 0.1, 0.5);
NodeRef ping = sine * env;

graph->play(ping);
graph->start();
graph->wait();
```

## Installation

### Dependencies

#### macOS

```
brew install python libsndfile libsoundio gsl
```

#### Linux, Raspberry Pi

```
apt-get install git cmake g++ python3-pip libasound2-dev libgsl0-dev libsndfile1-dev libsoundio-dev fftw3-dev 
```

If you experience an error on Raspberry Pi `libf77blas.so.3: cannot open shared object file`:

```
sudo apt-get install libatlas-base-dev
```

### Build (Python)

```
python3 setup.py build
python3 setup.py test
python3 setup.py install
```


### Build (C++)

```
mkdir build
cd build
cmake ..
make -j8
```

## Examples

See [examples](examples) for a number of example programs.

To run an example:
```
cd build
./hello-world
```

## Documentation

Documentation is in the works.

## Node classes

The following Node classes are currently available:

```
Abs
Add
AllpassDelay
AmplitudeToDecibels
AudioIn
AudioOut_Abstract
    AudioOut
    AudioOut_Dummy
BiquadFilter
BufferPlayer
BufferRecorder
ChannelArray
ChannelMixer
ChannelSelect
Clip
ClockDivider
CombDelay
Compressor
Constant
Counter
CrossCorrelate
DecibelsToAmplitude
Divide
EQ
Envelope
EnvelopeADSR
EnvelopeASR
Equal
Euclidean
FFT
FFTContinuousPhaseVocoder
FFTConvolve
FFTFindPeaks
FFTLPF
FFTPhaseVocoder
FFTTonality
FlipFlop
Fold
Gate
Granulator
GreaterThan
GreaterThanOrEqual
IFFT
If
Impulse
ImpulseSequence
Index
LFO
Latch
LessThan
LessThanOrEqual
Line
LinearPanner
Logistic
Maximiser
MidiNoteToFrequency
Modulo
MoogVCF
MouseDown
MouseX
MouseY
Multiply
NotEqual
OneTapDelay
OnsetDetector
Pow
RMS
Resample
RoundToScale
SVFFilter
SampleAndHold
Saw
SawLFO
ScaleLinExp
ScaleLinLin
Sine
SineLFO
Smooth
Square
SquareLFO
Squiz
StereoBalance
StereoWidth
StochasticNode
    PinkNoise
    RandomBrownian
    RandomCoin
    RandomExponential
    RandomExponentialDist
    RandomGaussian
    RandomImpulse
    RandomImpulseSequence
    RandomUniform
    WhiteNoise
Stutter
Subtract
Sum
Tanh
Triangle
TriangleLFO
WaveShaper
Wavetable
Wavetable2D
WetDry
Wrap
```

