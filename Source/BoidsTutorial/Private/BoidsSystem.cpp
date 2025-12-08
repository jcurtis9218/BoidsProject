// Fill out your copyright notice in the Description page of Project Settings.


#include "BoidsSystem.h"
#include <random>
#include "Boid.h"

// Sets default values
ABoidsSystem::ABoidsSystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABoidsSystem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABoidsSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<ABoid*> ABoidsSystem::get_boids()
{
	return _boids;
}

FVector ABoidsSystem::generate_initial_position()
{
	return FVector();
}

void ABoidsSystem::initialize_positions()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> x_range(-domain_size.X, domain_size.X);
	std::uniform_int_distribution<> y_range(-domain_size.Y, domain_size.Y);
	std::uniform_int_distribution<> z_range(-domain_size.Z, domain_size.Z);
	for (ABoid* boid : get_boids())
	{
		FVector initial_position = FVector(x_range(gen), y_range(gen), z_range(gen));
		boid->SetActorLocation(initial_position);
	}
}

FVector ABoidsSystem::generate_next_position(int boid_index, FVector center_of_mass)
{
	FVector v1, v2, v3, v4;
	v1 = seek_center_mass(boid_index, center_of_mass);
	v2 = maintain_distance(boid_index);
	v3 = match_nearby_velocity(boid_index);
	v4 = stay_in_bounds(boid_index);
	ABoid* boid = get_boids()[boid_index];
	boid->velocity += v1 + v2 + v3 + v4;
	return boid->GetActorLocation() + boid->velocity;
}

FVector ABoidsSystem::seek_center_mass(int boid_index, FVector center_of_mass)
{
	ABoid* boid = get_boids()[boid_index];
	return (center_of_mass - boid->GetActorLocation())/100.0;
}

FVector ABoidsSystem::maintain_distance(int boid_index)
{
	ABoid* boid = get_boids()[boid_index];
	FVector offset = FVector(0, 0, 0);
	for (ABoid* other_boid : get_boids())
	{
		if (other_boid == boid)
		{
			continue;
		}
		if ((boid->GetActorLocation() - other_boid->GetActorLocation()).Size() < nearby_distance)
		{
			offset -= (other_boid->GetActorLocation() - boid->GetActorLocation());
		}
	}
	return offset;
}

FVector ABoidsSystem::match_nearby_velocity(int boid_index)
{
	FVector offset = FVector(0, 0, 0);
	for (ABoid* other_boid : get_boids())
	{
		if (other_boid == get_boids()[boid_index])
		{
			continue;
		}
		offset += other_boid->velocity;
	}
	return offset/8.0;
}

FVector ABoidsSystem::stay_in_bounds(int boid_index)
{
	FVector offset = FVector(0, 0, 0);
	ABoid* boid = get_boids()[boid_index];
	FVector pos = boid->GetActorLocation();
	if (pos.X < -domain_size.X)
	{
		offset.X = border_force_strength;
	}
	else if (pos.X > domain_size.X)
	{
		offset.X = -border_force_strength;
	}
	if (pos.Y < -domain_size.Y)
	{
		offset.Y = border_force_strength;
	}
	else if (pos.Y > domain_size.Y)
	{
		offset.Y = -border_force_strength;
	}
	if (pos.Z < -domain_size.Z)
	{
		offset.Z = border_force_strength;
	}
	else if (pos.Z > domain_size.Z)
	{
		offset.Z = -border_force_strength;
	}
	return offset;
}

TArray<FVector> ABoidsSystem::generate_next_positions()
{
	TArray<FVector> next_positions;
	FVector center_of_mass;
	for (ABoid* boid : _boids)
	{
		center_of_mass += boid->GetActorLocation();
	}
	center_of_mass /= get_boids().Num();
	for (int i = 0; i < get_boids().Num(); i++)
	{
		next_positions.Add(generate_next_position(i, center_of_mass));
	}
	return next_positions;
}

void ABoidsSystem::update_positions()
{
	TArray<FVector> next_positions = generate_next_positions();
	for (int i = 0; i < get_boids().Num(); i++)
	{
		get_boids()[i]->update_position_and_rotation(next_positions[i]);
	}
	
}

