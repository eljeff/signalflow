#pragma once

/**--------------------------------------------------------------------------------
 * @file ringbuffer.h
 * @brief RingBuffer is a single-producer, single-consumer ringbuffer.
 *
 *--------------------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum signalflow_interpolation_mode_t : unsigned int;

namespace signalflow
{

template <class T>
class RingBuffer
{
public:
    RingBuffer(int size);
    ~RingBuffer();

    void append(T value);
    void extend(T *ptr, int count);
    T get(double index);
    T operator[](double index) { return this->get(index); }

private:
    T *data = nullptr;
    int size;
    int position;
    signalflow_interpolation_mode_t interpolation_mode;
};

template <class T>
RingBuffer<T>::RingBuffer(int size)
{
    this->data = new T[size]();
    this->position = 0;
    this->size = size;
}

template <class T>
RingBuffer<T>::~RingBuffer()
{
    delete[] this->data;
}

template <class T>
void RingBuffer<T>::append(T value)
{
    this->data[this->position] = value;
    this->position = (this->position + 1) % this->size;
}

template <class T>
void RingBuffer<T>::extend(T *ptr, int count)
{
    for (int i = 0; i < count; i++)
        this->append(ptr[i]);
}

template <class T>
T RingBuffer<T>::get(double index)
{
    double frame = index + this->position;
    while (frame < 0)
    {
        frame += this->size;
    }
    frame = fmod(frame, this->size);

    double frame_frac = (frame - (int) frame);
    int frame_index = (int) frame;
    int next_frame_index = ((int) ceil(frame)) % size;

    T rv = ((1.0 - frame_frac) * data[frame_index]) + (frame_frac * data[next_frame_index]);

    return rv;
}

}
