#include <iostream>
#include <algorithm>
#include <fstream>
#include <random>

enum CardColor {
    GOLD,
    SILVER,
    BRONZE,
    BLACK,
    RED,
};

enum CardType {
    NUM,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    SQUARE,
};

std::string to_string(const CardColor color) {
    switch (color) {
        case GOLD:
            return "GOLD";
        case SILVER:
            return "SILVER";
        case BRONZE:
            return "BRONZE";
        case BLACK:
            return "BLACK";
        case RED:
            return "RED";
    }
}

std::string to_string(const CardType type) {
    switch (type) {
        case NUM:
            return "NUM";
        case PLUS:
            return "+";
        case MINUS:
            return "-";
        case MULTIPLY:
            return "x";
        case DIVIDE:
            return "÷";
        case SQUARE:
            return "√";
    }
}

struct MathCard {
    unsigned short int number;
    CardColor color;
    CardType type;

    MathCard() = delete;
    MathCard(unsigned short int number, CardColor color, CardType type)
    : number(number), color(color), type(type) {}
};

class Game {
    int num_player;
    std::vector<MathCard> m_cards;
    std::vector<std::vector<MathCard>> m_players_cards;
    std::vector<MathCard> m_players_hidden_card;

    int m_round;
    int m_player_turn;
    int m_deck_index;

    const std::vector<MathCard>& get_player_cards(const int player_id);
    std::vector<MathCard>& get_mutable_player_cards(const int player_id);
    void init_deal();
public:
    Game(int num_player) : num_player(num_player) {
        m_player_turn = 0;
        m_deck_index = 0;
        m_round = 0;
        m_players_hidden_card.reserve(num_player);
    }
    void init(int player_turn);
    void deal();
    void write_player_cards(int player_id);
    bool has_type(int player_id, CardType type);
    int get_round () { return m_round; }
    void advance_round() { m_round++; }
    void print_public_cards();

    int get_player_turn() { return m_player_turn; }
};

const std::vector<MathCard>& Game::get_player_cards(const int player_id) {
    return m_players_cards.at(player_id);
}

std::vector<MathCard>& Game::get_mutable_player_cards(const int player_id) {
    return m_players_cards.at(player_id);
}

void Game::print_public_cards() {
    for (int i = 0; i < num_player; i++) {
        std::cout << "Player " << i << ": ";
        for (const MathCard& card: get_player_cards(i)) {
            if (card.type != NUM) {
                std::cout << to_string(card.type) << "\t";
            } else {
                std::cout << card.number << " " << to_string(card.color) << "\t";
            }
        }
        std::cout << "*Hidden*" << std::endl;
    }
}

void Game::write_player_cards(const int player_id) {
    std::string player_file = "player_" + std::to_string(player_id) + ".txt";
    std::ofstream file(player_file);

    file << "Hidden: ";
    MathCard hiddenCard = m_players_hidden_card.at(player_id);
    if (hiddenCard.type != NUM) {
        file << to_string(hiddenCard.type) << std::endl;
    } else {
        file << hiddenCard.number << "\t" << to_string(hiddenCard.color) << std::endl;
    }

    for (const MathCard& card: get_player_cards(player_id)) {
        if (card.type != NUM) {
            file << to_string(card.type) << std::endl;
        } else {
            file << card.number << "\t" << to_string(card.color) << std::endl;
        }
    }
    file.close();
}

bool Game::has_type(const int player_id, CardType type) {
    const std::vector<MathCard>& player_cards = get_player_cards(player_id);
    for (const MathCard& card: player_cards) {
        if (card.type == type) {
            return true;
        }
    }
    return false;
}

void Game::init_deal() {
    for (int i = 0; i < num_player; i++) {
        MathCard card = m_cards.at(m_deck_index);
        m_deck_index++;
        while(card.type != NUM) {
            card = m_cards.at(m_deck_index);
            m_deck_index++;
        }
        m_players_hidden_card.push_back(card);
    }
}

void Game::init(const int player_turn) {
    // Init players cards
    m_players_cards.reserve(num_player);
    std::vector<MathCard> init_cards;
    for (const CardType type: {PLUS, MINUS, DIVIDE}) {
        init_cards.emplace_back(-1, RED, type);
    }

    for (int i = 0; i < num_player; i++) {
        for (const CardType type: {PLUS, MINUS, DIVIDE}) {
            m_players_cards.emplace_back(init_cards);
        }
    }

    // Init deck cards
    m_cards.reserve(52); // 44 number cards, 4 multiply, 4 square
    for (const CardColor color: {GOLD, SILVER, BRONZE, BLACK}) {
        for (unsigned short int number = 0; number <= 10; number++) {
            m_cards.emplace_back(number, color, NUM);
        }
    }

    for (const CardType type: {MULTIPLY, SQUARE}) {
        for (int i = 0; i < 4; i++) {
            m_cards.emplace_back(-1, RED, type);
        }
    }

    // Shuffle cards
    std::random_device rd;
    std::mt19937 g(rd());

    std::ranges::shuffle(m_cards, g);

    // deal the first round, only num card is allowed
    init_deal();

    // write player cards
    for (int i = 0; i < num_player; i++) {
        write_player_cards(i);
    }
}

void Game::deal() {
    MathCard card = m_cards.at(m_deck_index);
    m_deck_index++;

    // Consider the cases where we need to re-deal
    // 1. The first card drawn cannot be a multiply card
    while (card.type == MULTIPLY && m_players_cards.at(m_player_turn).size() == 3) {
        // re-deal
        std::cout << "Re-deal" << std::endl;
        card = m_cards.at(m_deck_index);
        m_deck_index++;
    }

    // 2. If a player has a multiply or multiply card, the card drawn cannot be a multiply card
    while (card.type == MULTIPLY && (has_type(m_player_turn, MULTIPLY) || has_type(m_player_turn, SQUARE))) {
        // re-deal
        std::cout << "Re-deal" << std::endl;
        card = m_cards.at(m_deck_index);
        m_deck_index++;
    }

    // 3. If a player has a square or multiply card, the card drawn cannot be a square card
    while (card.type == SQUARE && (has_type(m_player_turn, SQUARE) || has_type(m_player_turn, MULTIPLY))) {
        // re-deal
        std::cout << "Re-deal" << std::endl;
        card = m_cards.at(m_deck_index);
        m_deck_index++;
    }

    // Add card to player's cards and write to file
    m_players_cards.at(m_player_turn).push_back(card);
    write_player_cards(m_player_turn);

    // Consider the case where we need to replace a card
    if (card.type == MULTIPLY) {
        auto& player_cards = get_mutable_player_cards(m_player_turn);
        std::cout << "Player" << m_player_turn << " draws a x card. Select a card to replace: (+,-,x): " << std::endl;
        std::string replace_card;
        std::cin >> replace_card;

        if (replace_card == "+") {
            player_cards.erase(std::ranges::remove_if(player_cards, [](MathCard& card) {
                return card.type == PLUS;
            }).begin(), player_cards.end());
        } else if (replace_card == "-") {
            player_cards.erase(std::ranges::remove_if(player_cards, [](MathCard& card) {
                return card.type == MINUS;
            }).begin(), player_cards.end());
        } else if (replace_card == "x") {
            player_cards.erase(std::ranges::remove_if(player_cards, [](MathCard& card) {
                return card.type == MULTIPLY;
            }).begin(), player_cards.end());
        }
    }
    write_player_cards(m_player_turn);

    // Consider the case where we need to add another card
    if (card.type == SQUARE || card.type == MULTIPLY) {
        MathCard additional_card = m_cards.at(m_deck_index);
        while (additional_card.type == SQUARE || additional_card.type == MULTIPLY) {
            additional_card = m_cards.at(m_deck_index);
            m_deck_index++;
        }
        m_players_cards.at(m_player_turn).push_back(additional_card);
        write_player_cards(m_player_turn);
    }

    // auto player_cards = get_player_cards(m_player_turn);

    m_player_turn = (m_player_turn + 1) % num_player;
    if (m_player_turn == 0) {
        m_round++;
    }
}

int main() {
    Game game(6);
    game.init(0);

    std::string cli_input;

    while (true) {
        std::cin >> cli_input;
        if (cli_input == "exit") {
            break;
        }

        if (cli_input != "n") {
            std::cout << "Not supported" << std::endl;
            continue;
        }

        game.deal();
        game.print_public_cards();

        if (game.get_round() == 2 && game.get_player_turn() == 0) {
            std::cout << "First round Bid" << std::endl;
        }

        if (game.get_round() == 3 && game.get_player_turn() == 0) {
            std::cout << "Second round Bid" << std::endl;
            break;
        }
    }

    std::cin >> cli_input;
    if (cli_input == "n") {
        std::cout << "Decide Bid 1 or 20 or both" << std::endl;
    }

    std::cin >> cli_input;
    if (cli_input == "n") {
        std::cout << "Show" << std::endl;
    }

    return 0;
}
