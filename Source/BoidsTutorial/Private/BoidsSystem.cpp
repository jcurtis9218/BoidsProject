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
	spawn_boids();
	initialize_positions();
	simulation_slowdown_counter = simulation_slowdown;
}

// Called every frame
void ABoidsSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	simulation_slowdown_counter -= 1;
	if (simulation_slowdown_counter <= 0)
	{
		update_positions();
		simulation_slowdown_counter = simulation_slowdown;
	}
}

TArray<ABoid*>* ABoidsSystem::get_boids()
{
	return &_boids;
}

void ABoidsSystem::spawn_boids()
{
	for (int i = 0; i < boid_count; i++)
	{
		AActor* actor_in_world = GetWorld()->SpawnActor(boid_class);
		ABoid* boid = Cast<ABoid>(actor_in_world);
		get_boids()->Add(boid);
	}
}

void ABoidsSystem::initialize_positions()
{
	UE_LOG(LogTemp, Log, TEXT("ABoidsSystem::initialize_positions"));
	UE_LOG(LogTemp, Warning, TEXT("%d Boids Detected"), get_boids()->Num());
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> x_range(-domain_size.X, domain_size.X);
	std::uniform_int_distribution<> y_range(-domain_size.Y, domain_size.Y);
	std::uniform_int_distribution<> z_range(-domain_size.Z, domain_size.Z);
	std::uniform_int_distribution<> vel_range(-10, 10);
	for (ABoid* boid : *get_boids())
	{
		FVector initial_position = FVector(x_range(gen), y_range(gen), z_range(gen));
		boid->update_position_and_rotation(initial_position+GetActorLocation());
		boid->velocity = FVector(vel_range(gen)/10., vel_range(gen)/10., vel_range(gen)/10.);
	}
}

FVector ABoidsSystem::generate_next_position(int boid_index, FVector center_of_mass)
{
	FVector v1, v2, v3, v4;
	v1 = seek_center_mass(boid_index);
	v2 = maintain_distance(boid_index);
	v3 = match_nearby_velocity(boid_index);
	v4 = stay_in_bounds(boid_index);
	ABoid* boid = (*get_boids())[boid_index];
	boid->velocity += v1 + v2 + v3 + v4;
	if (boid->velocity.Size() > max_speed)
	{
		boid->velocity = boid->velocity*max_speed/boid->velocity.Size();
	}
	return boid->GetActorLocation() + boid->velocity;
}

FVector ABoidsSystem::seek_center_mass(int boid_index)
{
	ABoid* boid = (*get_boids())[boid_index];
	FVector center_of_mass = FVector(0, 0, 0);
	for (int i = 0; i < get_boids()->Num(); i++)
	{
		if (i == boid_index)
		{
			continue;
		}
		ABoid* other_boid = (*get_boids())[i];
		center_of_mass += other_boid->GetActorLocation();
		
	}
	center_of_mass /= get_boids()->Num()-1;
	return (center_of_mass - boid->GetActorLocation())/100.0;
}

FVector ABoidsSystem::maintain_distance(int boid_index)
{
	ABoid* boid = (*get_boids())[boid_index];
	FVector offset = FVector(0, 0, 0);
	for (ABoid* other_boid : *get_boids())
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
	ABoid* boid = (*get_boids())[boid_index];
	for (int i = 0; i < get_boids()->Num(); i++)
	{
		if (i == boid_index)
		{
			continue;
		}
		ABoid* other_boid = (*get_boids())[i];
		offset += other_boid->velocity;
	}
	offset /= get_boids()->Num()-1;
	return (offset - boid->velocity)/8.;
}

FVector ABoidsSystem::stay_in_bounds(int boid_index)
{
	if (border_force_strength < 0)
	{
		return FVector(0, 0, 0);
	}
	FVector offset = FVector(0, 0, 0);
	ABoid* boid = (*get_boids())[boid_index];
	FVector pos = boid->GetActorLocation();
	if (pos.X < -domain_size.X+GetActorLocation().X)
	{
		offset.X = border_force_strength;
	}
	else if (pos.X > domain_size.X+GetActorLocation().X)
	{
		offset.X = -border_force_strength;
	}
	if (pos.Y < -domain_size.Y+GetActorLocation().Y)
	{
		offset.Y = border_force_strength;
	}
	else if (pos.Y > domain_size.Y+GetActorLocation().Y)
	{
		offset.Y = -border_force_strength;
	}
	if (pos.Z < -domain_size.Z+GetActorLocation().Z)
	{
		offset.Z = border_force_strength;
	}
	else if (pos.Z > domain_size.Z+GetActorLocation().Z)
	{
		offset.Z = -border_force_strength;
	}
	return offset;
}

TArray<FVector> ABoidsSystem::generate_next_positions()
{
	TArray<FVector> next_positions;
	FVector center_of_mass = FVector(0, 0, 0);
	for (ABoid* boid : _boids)
	{
		center_of_mass += boid->GetActorLocation();
	}
	center_of_mass /= get_boids()->Num();
	for (int i = 0; i < get_boids()->Num(); i++)
	{
		next_positions.Add(generate_next_position(i, center_of_mass));
	}
	return next_positions;
}

void ABoidsSystem::update_positions()
{
	for (int i = 0; i < get_boids()->Num(); i++)
	{
		FVector v1, v2, v3, v4;
		v1 = seek_center_mass(i);
		v2 = maintain_distance(i);
		v3 = match_nearby_velocity(i);
		v4 = stay_in_bounds(i);
		ABoid* boid = (*get_boids())[i];
		boid->velocity += v1 + v2 + v3 + v4;
		if (boid->velocity.Size() > max_speed)
		{
			boid->velocity = boid->velocity*max_speed/boid->velocity.Size();
			boid->update_position_and_rotation(boid->GetActorLocation()+boid->velocity);
		}
	}
	
}

