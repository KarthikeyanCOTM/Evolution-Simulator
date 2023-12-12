#include <iostream>
#include <limits.h>
#include <algorithm>
#include <vector>
#include <map>
using namespace std;

//world width
#define W 40
//world height
#define H 40

//world enviroments
enum terrain {GROUND, POISON, FOOD, CORPSE};

//world and genome generation
enum generation {DEFAULT, RANDOM, USER};

//cardinal directions
enum direction {WEST, NORTH, EAST, SOUTH, SIT};

//Genome size
#define GENOME_SIZE 7

//Genome location names
enum genes {FITNESS, MOVEMENT, SENSES, BRAIN_POWER, VIOLENCE, REPRODUCTION, FOOD_TYPE};

//enums of gene types
enum movement {UNMOVING, MOVING};
enum sense {NO_SENSE, SMELL, SIGHT};
enum brain {NO_POWER, BASIC, NORMAL, COMPLEX};
enum aggression {PEACEFUL, DEFENSE, AGGRESSIVE};
enum reproduc {ASEXUAL, SEXUAL};
enum food {VEG, OMN, CAR};

// Initial population size for the algorithm
int popSize = 10;

//structure of an individual
//contains it's genome array, latitude and longitude, number, and if it has reproduced
struct individual {
    int genome[GENOME_SIZE];
    int lat = -1;
    int lon = -1;
    int creatureNum = -1;
    bool repro = false;
};

//structure of a global location
//has varables for local creatures, how many creatures are present, and the terrain
struct location {
    individual p1;
    individual p2;
    int occupants;
    int terrainType;
};

//world "Map"
map<pair<int, int>, location> worldMap;

int rand_num(int start, int end);
bool repeat(string s, char ch);
void mutatedGene(int (&genome)[GENOME_SIZE]);
individual create_gnome(individual creature, int i);
int cal_fitness(int genome[GENOME_SIZE]);
individual asexualRep(individual oldCreature, double mutationChance);
individual sexualRep(individual parent1, struct individual parent2, double mutationChance);
bool findPartner(individual creature, individual (&partner));
void simUtil(double mutationChance, int geneGen, int generations);
void move(vector<individual> (&population));
void move(individual (&creature));
void repopulate(vector<individual> (&population), vector<individual>(&newPopulation), double mutationChance);
individual findClosestCreature(individual creature);
vector<individual> findAllSurrounding(individual creature);
void generateWorld(int genType);

//Function to move every individual in the world
void move(vector<individual> (&population)) {
    int direction;
    individual newInd;
    for (int i = 0; i < population.size(); ++i) {
        if (population[i].genome[MOVEMENT] != UNMOVING) {
            direction = rand() % 4;
            if ((direction == WEST && population[i].lon == 0) || (direction == EAST && population[i].lon == W - 1) ||
                (direction == NORTH && population[i].lat == 0) || (direction == SOUTH && population[i].lat == H - 1)) {
                bool validMove = false;
                while (!validMove) {
                    direction = rand() % 4;
                    if ((direction == WEST && population[i].lon > 0) ||
                        (direction == EAST && population[i].lon < W - 1) ||
                        (direction == NORTH && population[i].lat > 0) ||
                        (direction == SOUTH && population[i].lat < H - 1)) {
                        validMove = true;
                    }
                }
            }
            if (population[i].genome[BRAIN_POWER] == BASIC && population[i].genome[SENSES] != NO_SENSE) {

                if (direction == WEST && worldMap[make_pair(population[i].lat, population[i].lon - 1)].terrainType == POISON) {
                    direction = EAST;
                } else if (direction == EAST && worldMap[make_pair(population[i].lat, population[i].lon + 1)].terrainType == POISON) {
                    direction = WEST;
                } else if (direction == NORTH &&
                           worldMap[make_pair((population[i].lat - 1), population[i].lon)].terrainType == POISON) {
                    direction = SOUTH;
                } else if (direction == SOUTH &&
                           worldMap[make_pair((population[i].lat + 1), population[i].lon)].terrainType == POISON) {
                    direction = NORTH;
                }
            } else if ((population[i].genome[BRAIN_POWER] == NORMAL && population[i].genome[SENSES] != NO_SENSE) ||
                            (population[i].genome[BRAIN_POWER] == COMPLEX && population[i].genome[SENSES] == SMELL)) {

                if (direction == WEST && worldMap[make_pair(population[i].lat, population[i].lon - 1)].terrainType == POISON) {
                    direction = EAST;
                } else if (direction == EAST && worldMap[make_pair(population[i].lat, population[i].lon + 1)].terrainType == POISON) {
                    direction = WEST;
                } else if (direction == NORTH &&
                           worldMap[make_pair(population[i].lat - 1, population[i].lon)].terrainType == POISON) {
                    direction = SOUTH;
                } else if (direction == SOUTH &&
                           worldMap[make_pair(population[i].lat + 1, population[i].lon)].terrainType == POISON) {
                    direction = NORTH;
                }
                if (direction == WEST && worldMap[make_pair(population[i].lat, population[i].lon - 1)].occupants != 1) {
                    direction = SIT;
                } else if (direction == EAST && worldMap[make_pair(population[i].lat, population[i].lon + 1)].occupants != 1) {
                    direction = SIT;
                } else if (direction == NORTH && worldMap[make_pair(population[i].lat - 1, population[i].lon)].occupants != 1) {
                    direction = SIT;
                } else if (direction == SOUTH && worldMap[make_pair(population[i].lat + 1, population[i].lon)].occupants != 1) {
                    direction = SIT;
                }
            } else if (population[i].genome[BRAIN_POWER] == COMPLEX && population[i].genome[SENSES] != NO_SENSE) {//complex processing logic
                //finds the closest creature if there is one and moves accordingly
                individual foundInd = findClosestCreature(population[i]);
                if (population[i].genome[VIOLENCE] == 2) {
                    if (foundInd.lon != -1) {
                        if (foundInd.lat < foundInd.lon) {
                            if (foundInd.lon < population[i].lon) {
                                direction = WEST;
                            }else {
                                direction = EAST;
                            }
                        }else {
                            if (foundInd.lat < population[i].lat) {
                                direction = SOUTH;
                            }else {
                                direction = NORTH;
                            }
                        }
                    }
                    if (worldMap[make_pair(population[i].lat, population[i].lon - 1)].terrainType == POISON) {
                        direction = EAST;
                    } else if (worldMap[make_pair(population[i].lat, population[i].lon + 1)].terrainType == POISON) {
                        direction = WEST;
                    } else if (worldMap[make_pair((population[i].lat - 1), population[i].lon)].terrainType == POISON) {
                        direction = SOUTH;
                    } else if (worldMap[make_pair((population[i].lat + 1), population[i].lon)].terrainType == POISON) {
                        direction = NORTH;
                    }
                    if (worldMap[make_pair(population[i].lat, population[i].lon - 1)].terrainType == FOOD ||
                        worldMap[make_pair(population[i].lat, population[i].lon - 1)].terrainType == CORPSE) {

                        direction = WEST;
                    } else if (worldMap[make_pair(population[i].lat, population[i].lon + 1)].terrainType == FOOD ||
                               worldMap[make_pair(population[i].lat, population[i].lon - 1)].terrainType == CORPSE) {

                        direction = EAST;
                    } else if (worldMap[make_pair((population[i].lat - 1), population[i].lon)].terrainType == FOOD ||
                               worldMap[make_pair(population[i].lat, population[i].lon - 1)].terrainType == CORPSE) {

                        direction = NORTH;
                    } else if (worldMap[make_pair((population[i].lat + 1), population[i].lon)].terrainType == FOOD ||
                               worldMap[make_pair(population[i].lat, population[i].lon - 1)].terrainType == CORPSE) {

                        direction = SOUTH;
                    }
                }else {
                    if (worldMap[make_pair(population[i].lat, population[i].lon - 1)].terrainType == POISON ||
                            worldMap[make_pair(population[i].lat, population[i].lon - 1)].terrainType == CORPSE) {

                        direction = EAST;
                    }
                    if (worldMap[make_pair(population[i].lat, population[i].lon + 1)].terrainType == POISON ||
                            worldMap[make_pair(population[i].lat, population[i].lon + 1)].terrainType == CORPSE) {

                        if (direction == EAST) {
                            direction = SIT;
                        } else {
                            direction = WEST;
                        }
                    }
                    if (worldMap[make_pair((population[i].lat - 1), population[i].lon)].terrainType == POISON ||
                            worldMap[make_pair(population[i].lat - 1, population[i].lon)].terrainType == CORPSE) {

                        direction = SOUTH;
                    }if (worldMap[make_pair((population[i].lat + 1), population[i].lon)].terrainType == POISON ||
                            worldMap[make_pair(population[i].lat + 1, population[i].lon)].terrainType == CORPSE) {

                        if (direction == SOUTH) {
                            direction = SIT;
                        } else {
                            direction = NORTH;
                        }
                    }
                    if (worldMap[make_pair(population[i].lat, population[i].lon)].terrainType == FOOD) {
                        direction = SIT;
                    } else if (worldMap[make_pair(population[i].lat, population[i].lon - 1)].terrainType == FOOD) {
                        direction = WEST;
                    } else if (worldMap[make_pair(population[i].lat, population[i].lon + 1)].terrainType == FOOD) {
                        direction = EAST;
                    } else if (worldMap[make_pair((population[i].lat - 1), population[i].lon)].terrainType == FOOD) {
                        direction = NORTH;
                    } else if (worldMap[make_pair((population[i].lat + 1), population[i].lon)].terrainType == FOOD) {
                        direction = SOUTH;
                    }
                    if (foundInd.creatureNum != -1) {
                        vector<individual> nearCreatures = findAllSurrounding(population[i]);
                        vector<individual> newLocationCreatures;
                        if (direction == WEST) {
                            population[i].lon--;
                            newLocationCreatures = findAllSurrounding(population[i]);
                            population[i].lon++;
                        } else if (direction == EAST) {
                            population[i].lon++;
                            newLocationCreatures = findAllSurrounding(population[i]);
                            population[i].lon--;
                        } else if (direction == NORTH) {
                            population[i].lat++;
                            newLocationCreatures = findAllSurrounding(population[i]);
                            population[i].lat--;
                        } else if (direction == SOUTH) {
                            population[i].lat--;
                            newLocationCreatures = findAllSurrounding(population[i]);
                            population[i].lat++;
                        }
                        if (nearCreatures.size() > newLocationCreatures.size()) {
                            direction = SIT;
                        }
                    }
                }
            }
            if (direction == WEST && worldMap[make_pair(population[i].lat, (population[i].lon - 1))].occupants < 2) {

                worldMap[make_pair(population[i].lat, population[i].lon)].occupants--;
                population[i].lon--;
                worldMap[make_pair(population[i].lat, population[i].lon)].occupants++;
                if (worldMap[make_pair(population[i].lat, population[i].lon++)].p1.creatureNum == population[i].creatureNum) {
                    worldMap[make_pair(population[i].lat, population[i].lon++)].p1 = newInd;
                    worldMap[make_pair(population[i].lat, population[i].lon)].p1 = population[i];
                }else {
                    worldMap[make_pair(population[i].lat, population[i].lon++)].p2 = newInd;
                    worldMap[make_pair(population[i].lat, population[i].lon)].p2 = population[i];
                }

            } else if (direction == NORTH && worldMap[make_pair((population[i].lat-1), population[i].lon)].occupants < 2) {

                worldMap[make_pair(population[i].lat, population[i].lon)].occupants--;
                population[i].lat--;
                worldMap[make_pair(population[i].lat, population[i].lon)].occupants++;
                if (worldMap[make_pair(population[i].lat++, population[i].lon)].p1.creatureNum == population[i].creatureNum) {
                    worldMap[make_pair(population[i].lat++, population[i].lon)].p1 = newInd;
                    worldMap[make_pair(population[i].lat, population[i].lon)].p1 = population[i];
                }else {
                    worldMap[make_pair(population[i].lat++, population[i].lon)].p2 = newInd;
                    worldMap[make_pair(population[i].lat, population[i].lon)].p2 = population[i];
                }

            } else if (direction == EAST && worldMap[make_pair((population[i].lat - 1), population[i].lon)].occupants < 2) {

                worldMap[make_pair(population[i].lat, population[i].lon)].occupants--;
                population[i].lon++;
                worldMap[make_pair(population[i].lat, population[i].lon)].occupants++;
                if (worldMap[make_pair(population[i].lat, population[i].lon--)].p1.creatureNum == population[i].creatureNum) {
                    worldMap[make_pair(population[i].lat, population[i].lon--)].p1 = newInd;
                    worldMap[make_pair(population[i].lat, population[i].lon)].p1 = population[i];
                }else {
                    worldMap[make_pair(population[i].lat, population[i].lon--)].p2 = newInd;
                    worldMap[make_pair(population[i].lat, population[i].lon)].p2 = population[i];
                }

            } else if (direction == SOUTH && worldMap[make_pair((population[i].lat - 1), population[i].lon)].occupants < 2) {

                worldMap[make_pair(population[i].lat, population[i].lon)].occupants--;
                population[i].lat++;
                worldMap[make_pair(population[i].lat, population[i].lon)].occupants++;
                if (worldMap[make_pair(population[i].lat++, population[i].lon--)].p1.creatureNum ==
                    population[i].creatureNum) {
                    worldMap[make_pair(population[i].lat++, population[i].lon)].p1 = newInd;
                    worldMap[make_pair(population[i].lat, population[i].lon)].p1 = population[i];
                } else {
                    worldMap[make_pair(population[i].lat++, population[i].lon)].p2 = newInd;
                    worldMap[make_pair(population[i].lat, population[i].lon)].p2 = population[i];
                }
            }
        }
    }
}

//move function for a single creature
//used when placing the new population in the world when parent space is full
void move(individual (&creature)) {
    int direction = rand() % 4;
    if (direction == WEST && worldMap[make_pair(creature.lat, creature.lon - 1)].terrainType == POISON) {
        direction = EAST;
    } else if (direction == EAST && worldMap[make_pair(creature.lat, creature.lon + 1)].terrainType == POISON) {
        direction = WEST;
    } else if (direction == NORTH && worldMap[make_pair(creature.lat - 1, creature.lon)].terrainType == POISON) {
        direction = SOUTH;
    } else if (direction == SOUTH && worldMap[make_pair(creature.lat + 1, creature.lon)].terrainType == POISON) {
        direction = NORTH;
    }
    if (direction == WEST && worldMap[make_pair(creature.lat, creature.lon - 1)].occupants < 2) {

        creature.lon--;
        worldMap[make_pair(creature.lat, creature.lon)].occupants++;
        if (worldMap[make_pair(creature.lat, creature.lon)].occupants == 1) {
            worldMap[make_pair(creature.lat, creature.lon)].p1 = creature;
        }else {
            worldMap[make_pair(creature.lat, creature.lon)].p2 = creature;
        }

    } else if (direction == NORTH && worldMap[make_pair(creature.lat - 1, creature.lon)].occupants < 2) {

        creature.lat--;
        worldMap[make_pair(creature.lat, creature.lon)].occupants++;
        if (worldMap[make_pair(creature.lat, creature.lon)].occupants == 1) {
            worldMap[make_pair(creature.lat, creature.lon)].p1 = creature;
        }else {
            worldMap[make_pair(creature.lat, creature.lon)].p2 = creature;
        }

    } else if (direction == EAST && worldMap[make_pair(creature.lat - 1, creature.lon)].occupants < 2) {

        creature.lon++;
        worldMap[make_pair(creature.lat, creature.lon)].occupants++;
        if (worldMap[make_pair(creature.lat, creature.lon)].occupants == 1) {
            worldMap[make_pair(creature.lat, creature.lon)].p1 = creature;
        }else {
            worldMap[make_pair(creature.lat, creature.lon)].p2 = creature;
        }

    } else if (direction == SOUTH && worldMap[make_pair(creature.lat - 1, creature.lon)].occupants < 2) {

        creature.lat++;
        worldMap[make_pair(creature.lat, creature.lon)].occupants++;
        if (worldMap[make_pair(creature.lat, creature.lon)].occupants == 1) {
            worldMap[make_pair(creature.lat, creature.lon)].p1 = creature;
        }else {
            worldMap[make_pair(creature.lat, creature.lon)].p2 = creature;
        }

    }
}

// Function to find the closest creature to given
// returns a blank individual if closest is same tile or no creature in range
individual findClosestCreature(individual creature) {
    individual foundCreature;
    if (worldMap[make_pair(creature.lat, creature.lon)].occupants == 2) {
        return foundCreature;
    }
    for (int x = -2; x < 3; x++) {
        for (int y = -2; y < 3; y++) {
            if ((creature.lat + x >= 0 && creature.lon + y >= 0) && (creature.lat + x < H && creature.lon + y < W)) {
                if (worldMap[make_pair(creature.lat+x, creature.lon+y)].occupants != 0) {
                    foundCreature = worldMap[make_pair((creature.lat+x), (creature.lon+y))].p1;
                    return foundCreature;
                }
            }
        }
    }
    return foundCreature;
}

//finds all of the creatures up to two tiles from given creature
vector<individual> findAllSurrounding(individual creature) {
    vector<individual> foundCreatures;
    for (int x = -2; x < 3; x++) {
        for (int y = -2; y < 3; y++) {
            if ((creature.lat + x >= 0 && creature.lon + y >= 0) && (creature.lat + x < H && creature.lon + y < W)) {
                if (worldMap[make_pair(creature.lat+x, creature.lon+y)].occupants != 0) {
                    if (x == 0 && y == 0) {
                        if (worldMap[make_pair((creature.lat+x), (creature.lon+y))].p1.creatureNum != creature.creatureNum) {
                            foundCreatures.push_back(worldMap[make_pair((creature.lat+x), (creature.lon+y))].p1);
                        } else {
                            foundCreatures.push_back(worldMap[make_pair((creature.lat+x), (creature.lon+y))].p2);
                        }
                    }
                    if (worldMap[make_pair(creature.lat+x, creature.lon+y)].occupants == 1) {
                        foundCreatures.push_back(worldMap[make_pair((creature.lat + x), (creature.lon + y))].p1);
                    }else {
                        foundCreatures.push_back(worldMap[make_pair((creature.lat + x), (creature.lon + y))].p1);
                        foundCreatures.push_back(worldMap[make_pair((creature.lat + x), (creature.lon + y))].p2);
                    }
                }
            }
        }
    }
    return foundCreatures;
}

// Function to return a random number
// from start and end
int rand_num(int start, int end)
{
    int r = end - start;
    int rnum = start + rand() % r;
    return rnum;
}

// Function to return a mutated GNOME
// Mutated GNOME is a string
// with a random interchange
// of two genes to create variation in species
void mutatedGene(int (&genome)[GENOME_SIZE])
{
    int r = rand_num(1, GENOME_SIZE);
    if (r == MOVEMENT || r == REPRODUCTION) {
        genome[r] = rand_num(0, 2);
    } else if (r == SENSES || r == VIOLENCE || r == FOOD_TYPE) {
        genome[r] = rand_num(0, 2);
    } else if (r == BRAIN_POWER) {
        genome[r] = rand_num(0, 3);
    }
}

// Function to return a valid GENOME
// required to create the population
// TODO: Add user input of starting genome
individual create_gnome(struct individual individual, int i)
{
    if (i == DEFAULT) {
        for (int i = 0; i < GENOME_SIZE; i++) {
            individual.genome[i] = 0;
        }
    }else if (i == RANDOM) {
        for (int i = 0; i < GENOME_SIZE; i++) {
            if (i == MOVEMENT || i == REPRODUCTION) {
                individual.genome[i] = rand_num(0, 1);
            } else if (i == SENSES || i == VIOLENCE || i == FOOD_TYPE) {
                individual.genome[i] = rand_num(0, 2);
            } else if (i == BRAIN_POWER) {
                individual.genome[i] = rand_num(0, 3);
            }
        }
    }else if (i == USER) {

    }
    return individual;
}

// Function to return the fitness value of a gnome.
// The fitness value is the path length
// of the path represented by the GNOME.
int cal_fitness(int genome[GENOME_SIZE])
{
    genome[FITNESS] = 0;
    for (int i = 1; i < GENOME_SIZE; i++) {
        genome[FITNESS] += genome[i];
    }
    return genome[FITNESS];
}

//Asexual reproduction with only one creature
individual asexualRep(struct individual oldCreature, double mutationChance) {
    struct individual newCreature = oldCreature;
    mutatedGene(newCreature.genome);
    newCreature.genome[FITNESS] = cal_fitness(newCreature.genome);

    if (newCreature.genome[FITNESS] >= oldCreature.genome[FITNESS]) {
        return newCreature;
    } else {
        if (rand() / RAND_MAX > mutationChance) {
            return newCreature;
        } else {
            return oldCreature;
        }
    }
}

//Sexual reproduction between two creatures
individual sexualRep(individual parent1, individual parent2, double mutationChance) {
    individual new_ind;
    for (int i = 1; i < GENOME_SIZE; i++) {
        int diff = abs(parent1.genome[i] - parent2.genome[i]);
        if (diff == 0) {
            new_ind.genome[i] = parent1.genome[i];
        }else if (diff == 1) {
            if (parent1.genome[i] > parent2.genome[i]) {
                new_ind.genome[i] = parent1.genome[i];
            } else {
                new_ind.genome[i] = parent2.genome[i];
            }
        }else if (diff >= 2) {
            if (parent1.genome[i] > parent2.genome[i]) {
                new_ind.genome[i] = parent1.genome[i] - 1;
            } else {
                new_ind.genome[i] = parent2.genome[i] - 1;
            }
        }
    }
    individual mutated_ind = new_ind;
    mutatedGene(mutated_ind.genome);
    mutated_ind.genome[FITNESS] = cal_fitness(mutated_ind.genome);

    if (mutated_ind.genome[FITNESS] >= new_ind.genome[FITNESS]) {
        return mutated_ind;
    } else {
        if ((double)rand() / RAND_MAX > mutationChance) {
            return mutated_ind;
        } else {
            return new_ind;
        }
    }
}

// Attempts to find a partner within 2 cells of an individual
bool findPartner(individual creature, individual (&partner)) {
    bool found = false;
    for (int x = -2; x < 3; x++) {
        for (int y = -2; y < 3; y++) {
            if ((creature.lat + x >= 0 && creature.lon + y >= 0) && (creature.lat + x < H && creature.lon + y < W)) {
                if (worldMap[make_pair(creature.lat+x, creature.lon+y)].occupants != 0) {
                    partner = worldMap[make_pair(creature.lat+x, creature.lon+y)].p1;
                    found = true;
                    return found;
                }
            }
        }
    }
    return found;
}

// repopulation function
void repopulate(vector<individual> (&population), vector<individual> (&newPopulation), double mutationChance) {
    while (newPopulation.size() < popSize) {
        if (population[0].repro) {
            for (auto & i : population) {
                i.repro = false;
            }
        }
        for (int i = 0; i < population.size(); i++) {
            individual &p1 = population[i];
            if (!p1.repro) {
                if (worldMap[make_pair(p1.lon, p1.lat)].occupants == 2) {
                    individual p2;
                    if (worldMap[make_pair(p1.lon, p1.lat)].p1.creatureNum == p1.creatureNum) {
                        p2 = worldMap[make_pair(p1.lon, p1.lat)].p2;
                    }else {
                        p2 = worldMap[make_pair(p1.lon, p1.lat)].p1;
                    }
                    if (p1.genome[REPRODUCTION] == ASEXUAL) {
                        newPopulation.push_back(asexualRep(p1, mutationChance));
                        if (worldMap[make_pair(p1.lon, p1.lat)].p1.creatureNum == p1.creatureNum) {
                            worldMap[make_pair(p1.lon, p1.lat)].p1.repro = true;
                        }else {
                            worldMap[make_pair(p1.lon, p1.lat)].p2.repro = true;
                        }
                        p1.repro = true;
                    }
                    if (!p1.repro && !p2.repro) {
                        newPopulation.push_back(sexualRep(p1, p2, mutationChance));
                    } else if (!p1.repro) {
                        bool found = findPartner(p1, p2);
                        if (found) {
                            newPopulation.push_back(sexualRep(p1, p2, mutationChance));
                        }
                    }
                } else {
                    individual newP;
                    if (p1.genome[REPRODUCTION] == ASEXUAL) {
                        newP = asexualRep(p1, mutationChance);
                        newPopulation.push_back(newP);
                    } else {
                        individual p2;
                        bool found = findPartner(p1, p2);
                        if (found) {
                            newPopulation.push_back(sexualRep(p1, p2, mutationChance));
                        }
                    }
                }
            }
        }
    }
}

// Main Sim loop
void simUtil(double mutationChance, int geneGen, int generations)
{
    // Generation Number
    int gen = 1;

    vector<individual> population;

    // Populating the GENOME pool.
    for (int i = 0; i < popSize; i++) {
        individual creature;
        creature = create_gnome(creature, geneGen);
        creature.genome[FITNESS] =  cal_fitness(creature.genome);
        population.push_back(creature);
        bool valid = false;
        int lat = rand_num(0, H - 1);
        int lon = rand_num(0, W - 1);
        while (!valid) {
            if (worldMap[make_pair(lat, lon)].occupants < 2) {
                worldMap[make_pair(lat, lon)].occupants++;
                population[i].lat = lat;
                population[i].lon = lon;
                if (worldMap[make_pair(lat, lon)].occupants == 1) {
                    worldMap[make_pair(lat, lon)].p1 = population[i];
                } else {
                    worldMap[make_pair(lat, lon)].p2 = population[i];
                }
                valid = true;
            } else {
                lat = rand_num(0, H - 1);
                lon = rand_num(0, W - 1);
            }
        }
    }

    cout << "\nInitial population: " << endl
         << "GENOME\n";
    cout << "Movement Sense Brain   Violence    Repro   Food Fitness\n";
    for (int i = 0; i < popSize; i++) {
        for (int j = 0; j < GENOME_SIZE; j++) {
            cout << population[i].genome[j] << "        ";
        }
        cout << population[i].genome[FITNESS] << endl;
    }
    cout << "\n";

    bool found = false;

    // Iteration to perform
    // population crossing and gene mutation.
    while (gen <= generations) {
        move(population);
        for (int i = 0; i < population.size(); i++) {
            if (worldMap[make_pair(population[i].lat, population[i].lon)].terrainType == POISON) {
                population.erase(population.begin() + i);
                if (i != 0) {
                    i--;
                }
            }
            if (worldMap[make_pair(population[i].lat, population[i].lon)].occupants == 2 && population[i].genome[VIOLENCE] == AGGRESSIVE) {
                int x;
                for (x = 0; x < population.size(); x++) {
                    if (x != i) {
                        if (population[i].lon == population[x].lon && population[i].lat == population[x].lat) {
                            break;
                        }
                    }
                }
                if (population[x].genome[VIOLENCE] >= 1) {
                    if ((double)rand() / RAND_MAX > 0.5) {
                        population.erase(population.begin() + x);
                    }else {
                        population.erase(population.begin() + i);
                    }
                }
            }
        }
        vector<struct individual> newPopulation;

        repopulate(population, newPopulation, mutationChance);

        population.clear();
        for (const auto & i : newPopulation) {
            population.push_back(i);
        }
        individual blankInd;
        for (int i = 0; i < H; i++) {
            for (int j = 0; j < W; j++) {
                worldMap[make_pair(i, j)].occupants = 0;
                worldMap[make_pair(i, j)].p1 = blankInd;
                worldMap[make_pair(i, j)].p2 = blankInd;
            }
        }
        for (auto & i : population) {
            if (worldMap[make_pair(i.lat, i.lon)].occupants < 2) {
                worldMap[make_pair(i.lat, i.lon)].occupants++;
                if (worldMap[make_pair(i.lat, i.lon)].occupants == 1) {
                    worldMap[make_pair(i.lat, i.lon)].p1 = i;
                }else {
                    worldMap[make_pair(i.lat, i.lon)].p2 = i;
                }
            } else {
                move(i);
            }
        }
        cout << "Generation " << gen << " \n";
        cout << "GNOME\n";
        cout << "Movement Sense Brain   Violence    Repro    Food Fitness\n";

        for (auto & i : population) {
            for (int j = 1; j < GENOME_SIZE; j++) {
                cout << i.genome[j] << "        ";
            }
            cout << i.genome[FITNESS] << endl;
        }
        gen++;
    }
}

void generateWorld(int genType) {
    if (genType == DEFAULT) {
        for (int i = 0; i < W; ++i) {
            for (int j = 0; j < H; ++j) {
                worldMap[make_pair(i, j)].terrainType = GROUND;
            }
        }
    }else if (genType == RANDOM) {
        for (int i = 0; i < W; ++i) {
            for (int j = 0; j < H; ++j) {
                worldMap[make_pair(i, j)].terrainType = rand_num(GROUND, CORPSE);
            }
        }
    }else if (genType == USER) {

    }
}

int main()
{
    int worldGen;
    double mutationChance;
    int geneGen;
    int generations;
    srand(time(0));
    cout << "How would you like world generation:\n"
            "0 - No obstacles\n"
            "1 - Random obstacles\n";
    cin >> worldGen;
    generateWorld(worldGen);
    cout << "Give a decimal for mutation chance (0.5):\n";
    cin >> mutationChance;
    cout << "How would you like starting gene generation:\n"
            "0 - All base\n"
            "1 - Random\n";
    cin >> geneGen;
    cout << "Give an integer number for the minimum population size:\n";
    cin >> popSize;
    cout << "How many generation do you want(give an integer number):\n";
    cin >> generations;
    simUtil(mutationChance, geneGen, generations);
}