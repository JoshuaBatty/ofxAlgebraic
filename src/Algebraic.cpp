//
//  Algebraic.cpp
//  jenMax
//
//  Created by Mitchell Nordine on 5/08/13.
//
//

#include "Algebraic.h"

/*
 
 ABOUT ALGEBRAIC
 ***************
 
 Algebraic will return f(value) where value is passed in through
 setup(float _value).
 
 Without any input, Algebraic will return 1.0f.
 
 Algebraic will take into account any information you give it.
 
 It includes support for:
    - Sine
    - Sawtooth
    - Square
    - RandomWalk
    - Random
    - Triangle

    - Gradient
    - Bezier
    - Exponential

The signal is modulated by the following (REMEMBER, each of these can be
modulated with another "Algebraic" object):
    - Amplitude
    - Frequency 

The "gradient" is added to the result (determined by entering the x & y
co-ordinates).

 */

//--------------------------------------------------------------------------------//
// SETUP ALGEBRAIC
//--------------------------------------------------------------------------------//

void Algebraic::setup(float _value)
{
    
    originalValue = _value;
    value = originalValue;
    
    return;
}

//--------------------------------------------------------------------------------//
// SET IS DMX MODE
//--------------------------------------------------------------------------------//

void Algebraic::setDMXMode(bool val)
{
    isDMXMode = val;
}
bool Algebraic::getIsDMXMode()
{
    return isDMXMode;
}

//--------------------------------------------------------------------------------//
// CALCULATE SAW
//--------------------------------------------------------------------------------//

float Algebraic::calculateSaw(){
    //Handles positive and negative frequencies
    float clamp;
    int shift;
    if (frequency >= 0.0) {
        clamp = 1.0;
        shift = -1;
    } else {
        clamp = -1.0;
        shift = 1;
    }
    
    float phase = value * frequency / pointX;
    float fmodded = fmod(phase, clamp);
    
    return fmodded * 2 + shift;
}

//--------------------------------------------------------------------------------//
// CALCULATE RAMP
//--------------------------------------------------------------------------------//

float Algebraic::calculateRamp(){
    //Handles positive and negative frequencies
    float clamp;
    int shift;
    if (frequency >= 0.0) {
        clamp = -1.0;
        shift = 1;
    } else {
        clamp = 1.0;
        shift = -1;
    }
    
    float phase = value * (frequency*-1) / pointX;
    float fmodded = fmod(phase, clamp);
    
    return fmodded * 2 + shift;
}

//--------------------------------------------------------------------------------//
// RETURN SINE
//--------------------------------------------------------------------------------//

float Algebraic::getSine()
{
    return getSignal(sin(TWO_PI * value * frequency / pointX));
}

//--------------------------------------------------------------------------------//
// RETURN TRIANGLE
//--------------------------------------------------------------------------------//

float Algebraic::getTriangle()
{
    float sinewave = sin(TWO_PI * value * frequency / pointX);
    
    float out;
    if(sinewave > 0.0){
        out = calculateRamp();
    } else if(sinewave < 0.0){
        out = calculateSaw();
    }
    return getSignal(out * 2 - 1);
}

//--------------------------------------------------------------------------------//
// RETURN SAW
//--------------------------------------------------------------------------------//

float Algebraic::getSaw()
{
    return getSignal(calculateSaw());
}

//--------------------------------------------------------------------------------//
// RETURN RAMP
//--------------------------------------------------------------------------------//

float Algebraic::getRamp()
{
    return getSignal(calculateRamp());
}

//--------------------------------------------------------------------------------//
// RETURN SQUARE
//--------------------------------------------------------------------------------//

float Algebraic::getSquare()
{
    return getSignal((sin(TWO_PI * value * frequency / pointX) >= 0) ? 1.f : -1.f);
}

//--------------------------------------------------------------------------------//
// RETURN RANDOM
//--------------------------------------------------------------------------------//

float Algebraic::getRandom()
{
    return getSignal(ofRandom(min, max));
}

//--------------------------------------------------------------------------------//
// RETURN NOISE WALK
//--------------------------------------------------------------------------------//

float Algebraic::getNoiseWalk()
{
    return getSignal(ofSignedNoise(value * frequency / pointX));
}

//--------------------------------------------------------------------------------//
// RETURN GAUSSIAN
//--------------------------------------------------------------------------------//

float Algebraic::getGaussian()
{
    
    float answer;
    float freq = JenGaussianAlt(frequency,0.01*frequency);
    
    //answer = sin(TWO_PI * value * freq / pointX);
    //answer = fmod(value * freq / pointX, 1.0f) * 2 - 1;
    answer = getSine();
    
    answer = getSignal(answer);
    
    return answer;
    
}

//--------------------------------------------------------------------------------//
// SET AMPLITUDE
//--------------------------------------------------------------------------------//

float Algebraic::setAmplitude(float _amplitude)
{
    return amplitude = _amplitude;
}

//--------------------------------------------------------------------------------//
// SET POSITION OFFSET
//--------------------------------------------------------------------------------//

float Algebraic::setPositionOffset(float _offset)
{
    return position_offset = _offset;
}

//--------------------------------------------------------------------------------//
// SET FREQUENCY
//--------------------------------------------------------------------------------//

float Algebraic::setFrequency(float _frequency)
{
    return frequency = _frequency;
}

float Algebraic::getFrequency(){
    return frequency;
}

//--------------------------------------------------------------------------------//
// GET VALUE MULTIPLIED BY AMPLITUDE
//--------------------------------------------------------------------------------//

float Algebraic::getWithAmplitude(float value)
{
    /* Multiplies value with amplitude */
    if(isDMXMode){
        float amp_offset = ofMap(position_offset,0.0,1.0,1.0,-1.0);
        return (value * (amplitude-position_offset)) - (amp_offset - (amplitude-position_offset)) ; // This is what i want for DMX
    } else {
        return value * amplitude;
    }
    
    /*
    if(value > 0.0){
        return ofMap(value, 0.0, 1.0, 0.5, 0.5 + (amplitude * 0.5));
    } else if(value < 0.0){
        return ofMap(value, 0.0, -1.0, 0.5, 0.5 - (amplitude * 0.5));
    }
    */
  //  return ofMap(value, -1.0, 1.0, min,max) * amplitude;
}

//--------------------------------------------------------------------------------//
// RETURN GRADIENT
//--------------------------------------------------------------------------------//

float Algebraic::setGradient(float _pointX, float _pointY)
{
    pointX = _pointX;
    pointY = _pointY;
    if (!max) max = pointX; // If no max, assign pointX
    return gradient = (float)pointY / (float)pointX;
}

//--------------------------------------------------------------------------------//
// GET SIGNAL
//--------------------------------------------------------------------------------//

float Algebraic::getSignal(float _value)
{
    return getWithAmplitude(_value) + getBezier();
}

float Algebraic::getSignal()
{
    return getBezier();
}

//--------------------------------------------------------------------------------//
// GET BEZIER
//--------------------------------------------------------------------------------//
/* This function uses the point used to determine the gradient, to instead
   interpolate along a bezier curve. The "x" value for the point influencing
   the curve is always halfway between 0.0f & pointX. It's "y" value is determined
   by the "BezierDepth" variable.
 */

float Algebraic::getBezier()
{
    
    /* Points needed for Bezier:
     float x1 = 0.0
     float x2 = pointX/2.0f
     float x3 = pointX
     float y1 = 0.0 */
     float y2 = (pointY/2.0f) + bezierDepth*(pointY/2.0f); /*
     float y3 = pointY
     */
    
    float valueRelative = value / pointX;
    
    /* Re-adjust linear trajectory for each step */
    //float xa = getBezierPt(0.0f, x2, value);
    float ya = getBezierPt(0.0f, y2, valueRelative);
    //float xb = getBezierPt(x2, pointX, value);
    float yb = getBezierPt(y2, pointY, valueRelative);
    
    /* Position */
    return getBezierPt(ya, yb, valueRelative);
}

//--------------------------------------------------------------------------------//

float Algebraic::getBezierPt(float n1, float n2, float perc)
{
    float difference = n2 - n1;
    return n1 + (difference * perc);
}

//--------------------------------------------------------------------------------//
// SET BEZIER DEPTH
//--------------------------------------------------------------------------------//

float Algebraic::setBezierDepth(float _depth)
{
    return bezierDepth = _depth;
}

//--------------------------------------------------------------------------------//
// SET MIN
//--------------------------------------------------------------------------------//

float Algebraic::setMin(float _min)
{
    return min = _min;
}

//--------------------------------------------------------------------------------//
// SET MAX
//--------------------------------------------------------------------------------//

float Algebraic::setMax(float _max)
{
    return max = _max;
}

//--------------------------------------------------------------------------------//
// SET MIN & MAX
//--------------------------------------------------------------------------------//

float Algebraic::setMinMax(float _min, float _max)
{
    min = _min;
    max = _max;
    return max - min;
}


//--------------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------------//



//--------------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------------//



//--------------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------------//
