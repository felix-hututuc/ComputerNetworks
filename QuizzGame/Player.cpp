//
// Created by felix on 04.12.2021.
//

#include "Player.h"

Player::Player(const std::string& username) {
    this->username = username;
    this->score = 0;
}

void Player::setScore(const unsigned int &newScore) {
    score = newScore;
}

const std::string& Player::getUsername() const {
    return username;
}

const unsigned int& Player::getScore() const {
    return score;
}

void Player::incScore() {
    score += 50;
}

std::pair<std::string, int> Player::makePair() {
    return std::pair<std::string, int> (username, score);
}