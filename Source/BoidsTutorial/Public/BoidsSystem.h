// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Boid.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoidsSystem.generated.h"

UCLASS()
class BOIDSTUTORIAL_API ABoidsSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoidsSystem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	int boid_count;
	
	UPROPERTY(EditAnywhere)
	float nearby_distance;

	UPROPERTY(EditAnywhere)
	FVector domain_size;

	UPROPERTY(EditAnywhere)
	float border_force_strength;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABoid> boid_class;
	
	TArray<ABoid*> _boids;
	TArray<ABoid*> get_boids();
	void spawn_boids();
	void initialize_positions();
	FVector generate_next_position(int boid_index, FVector center_of_mass);
	FVector seek_center_mass(int boid_index, FVector center_of_mass);
	FVector maintain_distance(int boid_index);
	FVector match_nearby_velocity(int boid_index);
	FVector stay_in_bounds(int boid_index);
	TArray<FVector> generate_next_positions();
	void update_positions();
};
