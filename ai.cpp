#include "ai_client.h"
#include "card.hpp"

#include <fstream>
#include <stdlib.h>

class AI : public AiClient
{
public:
    //选择初始卡组
    void chooseCards()
    {
        // artifacts和creatures可以修改
        my_artifacts = {"HolyLight"};
        my_creatures = {"Archer", "Swordsman", "Priest"};
        init();
    }

    void play()
    {
        if (round < 20)
            endRound();
        else
            exit(0);
    }
};

int main()
{
    std::ifstream datafile("Data.json");
    json all_data;
    datafile>>all_data;
    card::get_data_from_json(all_data);

    AI player_ai;
    player_ai.chooseCards();
    while (true)
    {
        player_ai.updateGameInfo();
        player_ai.play();
    }
    return 0;
}
