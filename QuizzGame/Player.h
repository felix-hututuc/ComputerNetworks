//
// Created by felix on 04.12.2021.
//

#include <iostream>
#include <string>

#ifndef QUIZZGAME_PLAYER_H
#define QUIZZGAME_PLAYER_H


class Player {
    std::string username;
    unsigned int score = 0;

public:
    Player() { score = 0; }
    Player(const std::string& username);
    void setScore(const unsigned int& newScore);
    const unsigned int& getScore() const ;
    const std::string& getUsername() const ;
    void incScore();
    std::pair<std::string, int> makePair();
};


#endif //QUIZZGAME_PLAYER_H
