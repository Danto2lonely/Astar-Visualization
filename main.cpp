#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <unordered_map>
#include <thread>

const int GRID_WIDTH = 20;
const int GRID_HEIGHT = 20;
const int CELL_SIZE = 30;

struct Node {
    int x, y;
    float gCost, hCost;
    Node* parent;

    Node(int x, int y, Node* parent = nullptr) : x(x), y(y), gCost(0), hCost(0), parent(parent) {}
    float fCost() const { return gCost + hCost; }
};

struct CompareNodes {
    bool operator()(const Node* a, const Node* b) { return a->fCost() > b->fCost(); }
};

// Function to visualize A* search step by step
std::vector<Node*> findPath(sf::Vector2i start, sf::Vector2i end, std::vector<std::vector<int>>& grid,
    std::vector<Node*>& exploredNodes, sf::RenderWindow& window) {
    std::priority_queue<Node*, std::vector<Node*>, CompareNodes> openSet;
    std::unordered_map<int, Node*> allNodes;

    auto getIndex = [](int x, int y) { return y * GRID_WIDTH + x; };
    auto heuristic = [](Node* a, Node* b) { return std::abs(a->x - b->x) + std::abs(a->y - b->y); };

    Node* startNode = new Node(start.x, start.y);
    Node* endNode = new Node(end.x, end.y);
    openSet.push(startNode);
    allNodes[getIndex(start.x, start.y)] = startNode;

    std::vector<sf::Vector2i> directions = { {0,1}, {1,0}, {0,-1}, {-1,0} };

    while (!openSet.empty()) {
        Node* current = openSet.top();
        openSet.pop();

        // Store explored nodes and update visualization
        exploredNodes.push_back(current);

        // Render the grid in real time
        window.clear();
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(static_cast<float>(x * CELL_SIZE), static_cast<float>(y * CELL_SIZE));
                cell.setOutlineThickness(1);
                cell.setOutlineColor(sf::Color::Black);

                if (grid[y][x] == 1)
                    cell.setFillColor(sf::Color::Black);  // Walls
                else if (sf::Vector2i(x, y) == start)
                    cell.setFillColor(sf::Color::Green);  // Start
                else if (sf::Vector2i(x, y) == end)
                    cell.setFillColor(sf::Color::Red);    // End
                else
                    cell.setFillColor(sf::Color::White);  // Default Empty Cells

                window.draw(cell);
            }
        }

        // Draw explored nodes in light blue
        for (Node* node : exploredNodes) {
            if (sf::Vector2i(node->x, node->y) != start && sf::Vector2i(node->x, node->y) != end) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(static_cast<float>(node->x * CELL_SIZE), static_cast<float>(node->y * CELL_SIZE));
                cell.setFillColor(sf::Color(173, 216, 230));
                window.draw(cell);
            }
        }

        window.display();
        sf::sleep(sf::milliseconds(50));  // Slow down visualization for step-by-step effect

        // Path found
        if (current->x == end.x && current->y == end.y) {
            std::vector<Node*> path;
            while (current) {
                path.push_back(current);
                current = current->parent;
            }
            return path;
        }

        // Expand neighbors
        for (const auto& dir : directions) {
            int newX = current->x + dir.x;
            int newY = current->y + dir.y;
            if (newX >= 0 && newX < GRID_WIDTH && newY >= 0 && newY < GRID_HEIGHT && grid[newY][newX] == 0) {
                int index = getIndex(newX, newY);
                if (allNodes.find(index) == allNodes.end()) {
                    Node* neighbor = new Node(newX, newY, current);
                    neighbor->gCost = current->gCost + 1.0f;
                    neighbor->hCost = heuristic(neighbor, endNode);
                    openSet.push(neighbor);
                    allNodes[index] = neighbor;
                }
            }
        }
    }
    return {};
}

int main() {
    sf::RenderWindow window(sf::VideoMode(GRID_WIDTH * CELL_SIZE, GRID_HEIGHT * CELL_SIZE), "A* Pathfinding Visualization");
    std::vector<std::vector<int>> grid(GRID_HEIGHT, std::vector<int>(GRID_WIDTH, 0));
    sf::Vector2i start(-1, -1), end(-1, -1);
    std::vector<Node*> path;
    std::vector<Node*> exploredNodes;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseButtonPressed) {
                int x = event.mouseButton.x / CELL_SIZE;
                int y = event.mouseButton.y / CELL_SIZE;
                if (event.mouseButton.button == sf::Mouse::Left) {
                    start = { x, y };
                }
                else if (event.mouseButton.button == sf::Mouse::Right) {
                    end = { x, y };
                }
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::W) {
                    int x = sf::Mouse::getPosition(window).x / CELL_SIZE;
                    int y = sf::Mouse::getPosition(window).y / CELL_SIZE;
                    grid[y][x] = 1;  // Mark wall
                }
                else if (event.key.code == sf::Keyboard::R && start.x != -1 && end.x != -1) {
                    exploredNodes.clear();  // Clear previous search steps
                    path = findPath(start, end, grid, exploredNodes, window);
                }
                else if (event.key.code == sf::Keyboard::C) {
                    grid.assign(GRID_HEIGHT, std::vector<int>(GRID_WIDTH, 0));
                    path.clear();
                    exploredNodes.clear();
                    start = end = { -1, -1 };
                }
            }
        }

        window.clear();
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(static_cast<float>(x * CELL_SIZE), static_cast<float>(y * CELL_SIZE));
                cell.setOutlineThickness(1);
                cell.setOutlineColor(sf::Color::Black);

                if (grid[y][x] == 1) cell.setFillColor(sf::Color::Black);
                else if (sf::Vector2i(x, y) == start) cell.setFillColor(sf::Color::Green);
                else if (sf::Vector2i(x, y) == end) cell.setFillColor(sf::Color::Red);
                else cell.setFillColor(sf::Color::White);

                window.draw(cell);
            }
        }

        // Draw final path in yellow
        for (Node* node : path) {
            if (sf::Vector2i(node->x, node->y) != start && sf::Vector2i(node->x, node->y) != end) {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(static_cast<float>(node->x * CELL_SIZE), static_cast<float>(node->y * CELL_SIZE));
                cell.setFillColor(sf::Color::Yellow);
                window.draw(cell);
            }
        }

        window.display();
    }
    return 0;
}
