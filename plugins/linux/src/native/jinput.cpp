/**
 * Copyright (C) 2003 Jeremy Booth (jeremy@newdawnsoftware.com)
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer. Redistributions in binary 
 * form must reproduce the above copyright notice, this list of conditions and 
 * the following disclaimer in the documentation and/or other materials provided
 * with the distribution. 
 * The name of the author may not be used to endorse or promote products derived
 * from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO 
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "net_java_games_input_LinuxDevice.h"
#include "net_java_games_input_LinuxEnvironmentPlugin.h"
#include "net_java_games_input_LinuxKeyboard.h"

#include "Device.h"
#include "EventDevice.h"
#include "JoystickDevice.h"
#include "MixedDevice.h"
#include "eventInterface.h"
#include "joystickInterface.h"

Device **jinputDeviceList;
int jinputNumDevices;

/*
 * Class:     net_java_games_input_LinuxEnvironmentPlugin
 * Method:    init
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_java_games_input_LinuxEnvironmentPlugin_init
  (JNIEnv *, jobject) {

  if(evInit()!=0) {
    fprintf(stderr, "Failed to init native event interface\n");
    return -1;
  }
  if(jsInit()!=0) {
    fprintf(stderr, "Failed to init native joystick interface\n");
    return -1;
  }

  int numEventDevices = evGetNumberDevices();
  EventDevice *eventDevices[numEventDevices];
  
  evGetDevices((Device **)eventDevices);

  int numJoysticks = jsGetNumberDevices();
  JoystickDevice *jsDevices[numJoysticks];
  jsGetDevices((Device **)jsDevices);

  
  int i;
  int j;
  int joystickPtr = 0;
  jinputDeviceList = (Device **)malloc(numEventDevices * sizeof(Device *));
  for(i=0;i<numEventDevices;i++) {
    EventDevice *eventDevice = eventDevices[i];
    int deviceCountCache = jinputNumDevices;

    for(j=joystickPtr;j<numJoysticks;j++) {
      JoystickDevice *jsDevice = jsDevices[j];
      if((jsDevice->getNumberButtons() == eventDevice->getNumberButtons()) && (jsDevice->getNumberAbsAxes() == (eventDevice->getNumberAbsAxes() + eventDevice->getNumberRelAxes()))) {
        const char *jsName = jsDevice->getName();
        const char *eventDeviceName = eventDevice->getName();

        if(strcmp(jsName, eventDeviceName) == 0) {
          // The current event device is the curre joystick device too
          jinputDeviceList[jinputNumDevices] = new MixedDevice(jsDevice, eventDevice);
          jinputNumDevices++;
          joystickPtr = j;
          j = numJoysticks;
        }
      }
      /*if(jinputNumDevices == deviceCountCache) {
        //fprintf(stderr, "event device \"%s\" doesn't match js \"%s\"\n", eventDevice->getName(), jsDevice->getName());
        //fprintf(stderr, "event device has %d rel axes, %d abs axis and %d buttons\n", eventDevice->getNumberRelAxes(), eventDevice->getNumberAbsAxes(), eventDevice->getNumberButtons());
        //fprintf(stderr, "js device has %d axes and %d buttons\n", jsDevice->getNumberAbsAxes(), jsDevice->getNumberButtons());
      } else {
        //fprintf(stderr, "event device %s did match js %s\n", eventDevice->getName(), jsDevice->getName());
      } */
  
    }

    if(jinputNumDevices == deviceCountCache) {
      jinputDeviceList[jinputNumDevices] = eventDevice;
      jinputNumDevices++;
    }
  }


  return(0);

}

/*
 * Class:     net_java_games_input_LinuxEnvironmentPlugin
 * Method:    getDeviceName
 * Signature: (I)Ljava/lang/String;
 */

JNIEXPORT jstring JNICALL Java_net_java_games_input_LinuxEnvironmentPlugin_getDeviceName
  (JNIEnv *env, jobject, jint deviceID) {
  
  return env->NewStringUTF(jinputDeviceList[deviceID]->getName());
}

/*
 * Class:     net_java_games_input_LinuxEnvironmentPlugin
 * Method:    getNumAbsAxes
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_net_java_games_input_LinuxEnvironmentPlugin_getNumAbsAxes
  (JNIEnv *env, jobject, jint deviceID) {

  return jinputDeviceList[deviceID]->getNumberAbsAxes();
}

/*
 * Class:     net_java_games_input_LinuxEnvironmentPlugin
 * Method:    getNumRelAxes
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_net_java_games_input_LinuxEnvironmentPlugin_getNumRelAxes
  (JNIEnv *env, jobject, jint deviceID) {
  
  return jinputDeviceList[deviceID]->getNumberRelAxes();
}

/*
 * Class:     net_java_games_input_LinuxEnvironmentPlugin
 * Method:    getNumButtons
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_net_java_games_input_LinuxEnvironmentPlugin_getNumButtons
  (JNIEnv *, jobject, jint deviceID) {

  return jinputDeviceList[deviceID]->getNumberButtons();
}

/*
 * Class:     net_java_games_input_LinuxEnvironmentPlugin
 * Method:    getNumberOfDevices
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_net_java_games_input_LinuxEnvironmentPlugin_getNumberOfDevices
  (JNIEnv *, jobject) {

  return jinputNumDevices;
}

/*
 * Class:     net_java_games_input_LinuxDevice
 * Method:    getNativeSupportedAbsAxes
 * Signature: (I[I)V
 */
JNIEXPORT void JNICALL Java_net_java_games_input_LinuxDevice_getNativeSupportedAbsAxes
  (JNIEnv *env, jobject, jint deviceID, jintArray axesData) {

  jint *axisReturns = env->GetIntArrayElements(axesData, 0);
  
  jinputDeviceList[deviceID]->getSupportedAbsAxes(axisReturns);

  env->ReleaseIntArrayElements(axesData, axisReturns, 0);
}

/*
 * Class:     net_java_games_input_LinuxDevice
 * Method:    getNativeSupportedRelAxes
 * Signature: (I[I)V
 */
JNIEXPORT void JNICALL Java_net_java_games_input_LinuxDevice_getNativeSupportedRelAxes
  (JNIEnv *env, jobject, jint deviceID, jintArray axesData) {

  jint *axisReturns = env->GetIntArrayElements(axesData, 0);
  
  jinputDeviceList[deviceID]->getSupportedRelAxes(axisReturns);

  env->ReleaseIntArrayElements(axesData, axisReturns, 0);
}

/*
 * Class:     net_java_games_input_LinuxDevice
 * Method:    getNativeSupportedButtons
 * Signature: (I[I)V
 */
JNIEXPORT void JNICALL Java_net_java_games_input_LinuxDevice_getNativeSupportedButtons
  (JNIEnv *env, jobject, jint deviceID, jintArray buttonData) {

  jint *buttonDataElements = env->GetIntArrayElements(buttonData, 0);
  
  jinputDeviceList[deviceID]->getSupportedButtons(buttonDataElements);

  env->ReleaseIntArrayElements(buttonData, buttonDataElements, 0);
}

/*
 * Class:     net_java_games_input_LinuxDevice
 * Method:    nativePoll
 * Signature: (I[I[I[I)I
 */
JNIEXPORT jint JNICALL Java_net_java_games_input_LinuxDevice_nativePoll
  (JNIEnv *env, jobject, jint deviceID, jintArray buttons, jintArray relAxes, jintArray absAxes) {

  jint *buttonElements = env->GetIntArrayElements(buttons, 0);
  jint *relAxesElements = env->GetIntArrayElements(relAxes, 0);
  jint *absAxesElements = env->GetIntArrayElements(absAxes, 0);

  int retval = jinputDeviceList[deviceID]->poll();
  jinputDeviceList[deviceID]->getPolledData(relAxesElements, absAxesElements, buttonElements);

  env->ReleaseIntArrayElements(buttons, buttonElements, 0);
  env->ReleaseIntArrayElements(relAxes, relAxesElements, 0);
  env->ReleaseIntArrayElements(absAxes, absAxesElements, 0);

  return retval;
}

/*
 * Class:     net_java_games_input_LinuxDevice
 * Method:    getNativeAbsAxisFuzz
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_net_java_games_input_LinuxDevice_getNativeAbsAxisFuzz
  (JNIEnv *, jobject, jint deviceID, jint axisID) {

  return jinputDeviceList[deviceID]->getAbsAxisFuzz(axisID);
}

/*
 * Class:     net_java_games_input_LinuxDevice
 * Method:    getNativeAbsAxisMaximum
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_net_java_games_input_LinuxDevice_getNativeAbsAxisMaximum
  (JNIEnv *, jobject, jint deviceID, jint axisID) {

  return jinputDeviceList[deviceID]->getAbsAxisMaximum(axisID);
}


/*
 * Class:     net_java_games_input_LinuxDevice
 * Method:    getNativeAbsAxisMinimum
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_net_java_games_input_LinuxDevice_getNativeAbsAxisMinimum
  (JNIEnv *, jobject, jint deviceID, jint axisID) {

  return jinputDeviceList[deviceID]->getAbsAxisMinimum(axisID);
}

/*
 * Class:     net_java_games_input_LinuxDevice
 * Method:    getNativePortType
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_net_java_games_input_LinuxDevice_getNativePortType
  (JNIEnv *, jobject, jint deviceID) {

  jinputDeviceList[deviceID]->getBusType();
}

/* Inaccessible static: NO_RUMBLERS */
/* Inaccessible static: _00024assertionsDisabled */
/* Inaccessible static: class_00024net_00024java_00024games_00024input_00024Keyboard */
/*
 * Class:     net_java_games_input_LinuxKeyboard
 * Method:    getNativeSupportedButtons
 * Signature: (I[I)V
 */
JNIEXPORT void JNICALL Java_net_java_games_input_LinuxKeyboard_getNativeSupportedButtons
  (JNIEnv *env, jobject, jint deviceID, jintArray buttons) {

  jint *buttonDataElements = env->GetIntArrayElements(buttons, 0);
  
  jinputDeviceList[deviceID]->getSupportedButtons(buttonDataElements);

  env->ReleaseIntArrayElements(buttons, buttonDataElements, 0);
}

/*
 * Class:     net_java_games_input_LinuxKeyboard
 * Method:    nativePoll
 * Signature: (I[I[I[I)I
 */
JNIEXPORT jint JNICALL Java_net_java_games_input_LinuxKeyboard_nativePoll
  (JNIEnv *env, jobject, jint deviceID, jintArray buttons, jintArray relAxes, jintArray absAxes) {

  jint *buttonElements = env->GetIntArrayElements(buttons, 0);
  jint *relAxesElements = env->GetIntArrayElements(relAxes, 0);
  jint *absAxesElements = env->GetIntArrayElements(absAxes, 0);

  int retval = jinputDeviceList[deviceID]->poll();
  jinputDeviceList[deviceID]->getPolledData(relAxesElements, absAxesElements, buttonElements);

  env->ReleaseIntArrayElements(buttons, buttonElements, 0);
  env->ReleaseIntArrayElements(relAxes, relAxesElements, 0);
  env->ReleaseIntArrayElements(absAxes, absAxesElements, 0);

  return retval;
}

/*
 * Class:     net_java_games_input_LinuxKeyboard
 * Method:    getNativePortType
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_net_java_games_input_LinuxKeyboard_getNativePortType
  (JNIEnv *, jobject, jint deviceID) {

  jinputDeviceList[deviceID]->getBusType();
}
