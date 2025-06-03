// Fill out your copyright notice in the Description page of Project Settings.


#include "PC_LegacyCameraShake.h"

UPC_LegacyCameraShake::UPC_LegacyCameraShake()
{
	OscillationDuration = 0.2f;     
	OscillationBlendInTime = 0.05f;  
	OscillationBlendOutTime = 0.05f;
	
	LocOscillation.X.Amplitude = 5.0f; 
	LocOscillation.X.Frequency = 50.0f;
	LocOscillation.Y.Amplitude = 5.0f; 
	LocOscillation.Y.Frequency = 50.0f;
	LocOscillation.Z.Amplitude = 5.0f; 
	LocOscillation.Z.Frequency = 50.0f;
}
