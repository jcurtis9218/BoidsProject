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
}

// Called every frame
void ABoidsSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	update_positions(DeltaTime);
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
	for (ABoid* boid : *get_boids())
	{
		FVector initial_position = FVector(x_range(gen), y_range(gen), z_range(gen));
		boid->update_position_and_rotation(initial_position+GetActorLocation());
		boid->velocity = FVector(rand()-(RAND_MAX/2), rand()-(RAND_MAX/2), rand()-(RAND_MAX/2)).GetSafeNormal()*max_speed;
		UE_LOG(LogTemp, Warning, TEXT("Initial Velocity: %f, %f, %f"), boid->velocity.X, boid->velocity.Y, boid->velocity.Z);
	}
}

FVector ABoidsSystem::generate_next_position(int boid_index, FVector center_of_mass, TArray<int> nearby_indices, float DeltaTime)
{
	FVector cohesion = seek_center_mass(boid_index, center_of_mass, nearby_indices.Num());
	FVector separation = maintain_distance(boid_index, nearby_indices);
	FVector alignment = match_nearby_velocity(boid_index, nearby_indices);
	FVector border_force = stay_in_bounds(boid_index);
	ABoid* boid = (*get_boids())[boid_index];
	FVector acceleration= (cohesion*cohesion_strength)+(separation*separation_strength)+(alignment*alignment_strength)+(border_force*border_force_strength);
	boid->velocity += acceleration*DeltaTime*time_scale;
	boid->velocity = boid->velocity.GetClampedToMaxSize(max_speed);
	
	return boid->GetActorLocation() + boid->velocity.GetSafeNormal()*DeltaTime*time_scale*max_speed;
}

FVector ABoidsSystem::seek_center_mass(int boid_index, FVector center_of_mass, int neighbor_count)
{
	if (neighbor_count == 0)
	{
		return FVector(0, 0, 0);
	}
	ABoid* boid = (*get_boids())[boid_index];
	return (center_of_mass - boid->GetActorLocation()).GetSafeNormal();
}

FVector ABoidsSystem::maintain_distance(int boid_index, TArray<int> nearby_indices)
{
	ABoid* boid = (*get_boids())[boid_index];
	FVector force = FVector(0, 0, 0);
	for (int i = 0; i < nearby_indices.Num(); i++)
	{
		FVector offset = boid->GetActorLocation() - (*get_boids())[nearby_indices[i]]->GetActorLocation();
		float distance = offset.Size();

		if (distance < separation_distance)
		{
			force += offset.GetSafeNormal()/distance;
		}
	}
	if (force.Size() > 0)
	{
		force = force.GetSafeNormal()*max_speed;
	}
	return force;
	
}

FVector ABoidsSystem::match_nearby_velocity(int boid_index, TArray<int> nearby_indices)
{
	ABoid* boid = (*get_boids())[boid_index];
	FVector nearby_average_velocity = FVector(0, 0, 0);
	for (int i = 0; i < nearby_indices.Num(); i++)
	{
		nearby_average_velocity += (*get_boids())[nearby_indices[i]]->velocity;
	}
	if (nearby_indices.Num() == 0)
	{
		return FVector(0, 0, 0);
	}
	nearby_average_velocity/=nearby_indices.Num();

	return nearby_average_velocity-boid->velocity;
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

TArray<FVector> ABoidsSystem::generate_next_positions(TArray<TArray<int>> neighbor_indices, float DeltaTime)
{
	TArray<FVector> next_positions;
	for (int i = 0; i < get_boids()->Num(); i++)
	{
		FVector center_mass = FVector(0, 0, 0);
		if (neighbor_indices[i].Num() > 0)
		{
			for (int neighbor : neighbor_indices[i])
			{
				center_mass += (*get_boids())[neighbor]->GetActorLocation();
			}
			center_mass /= neighbor_indices.Num();
		}
		next_positions.Add(generate_next_position(i, center_mass, neighbor_indices[i], DeltaTime));
	}
	return next_positions;
}

void ABoidsSystem::update_positions(float DeltaTime)
{
	TArray<TArray<int>> neighbor_indices;
	for (int i = 0; i < get_boids()->Num(); i++)
	{
		neighbor_indices.Add(TArray<int>());
		for (int j = 0; j < get_boids()->Num(); j++)
		{
			if (i == j)
			{
				continue;
			}
			if (((*get_boids())[i]->GetActorLocation() - (*get_boids())[j]->GetActorLocation()).Size() < nearby_distance)
			{
				neighbor_indices[i].Add(j);
			}
		}
	}
	TArray<FVector> next_positions = generate_next_positions(neighbor_indices, DeltaTime);
	int moved = 0;
	for (int i = 0; i < get_boids()->Num(); i++)
	{
		(*get_boids())[i]->update_position_and_rotation(next_positions[i]);
		moved++;
	}
	UE_LOG(LogTemp, Error, TEXT("%d Boids Moved"), moved);
}

