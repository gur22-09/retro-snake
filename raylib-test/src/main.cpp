#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <algorithm>
#include <deque>

using namespace std;

Color green = { 173, 204, 96, 255 };
Color darkGreen = { 43, 51, 24, 255 };

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime{};

bool isInDeque(const deque<Vector2>& deque, Vector2& pos) {
	for (unsigned int i = 0; i < deque.size(); ++i) {
		if (Vector2Equals(deque[i], pos)) {
			return true;
		}
	}

	return false;
}
 
bool isTimeElapsed(double& const interval) {
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval) {
        lastUpdateTime = currentTime;
        return true;
    }

    return false;
}
 
class Snake {
private:
    Rectangle segment{};
public:
    deque<Vector2> body{ Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} };
    Vector2 dir{ 1, 0 };
    bool addSegment{ false };
   

    void draw() {
        for (int i = 0; i < body.size(); ++i) {
            float x = body[i].x;
            float y = body[i].y;
   
			segment.x = offset + x * cellSize;
			segment.y = offset + y * cellSize;
			segment.width = (float)cellSize;
			segment.height = (float)cellSize;

            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
        }
    }

    void update() {
        body.push_front(Vector2Add(body[0], dir));
        
        if (addSegment) {
            addSegment = false;
        }
        else {
			body.pop_back();
        }
        
    }

   

    void reset() {
        body = { Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} };
        dir = { 1, 0 };
    }

};

class Food {
public:
    Vector2 position{ 5, 6 };
    Texture2D texture;

    Food(const char* imagePath, deque<Vector2>& snakeBody) {
        Image image = LoadImage(imagePath);
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = getRandomPos(snakeBody);
    }

    ~Food() {
        UnloadTexture(texture);
    }

    void draw() {
        DrawTexture(texture,offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 getRandomCell() {
		int x = GetRandomValue(0, cellCount - 1);
		int y = GetRandomValue(0, cellCount - 1);

        return { (float)x, (float)y };
    }

    Vector2 getRandomPos(deque<Vector2>& snakeBody) {
        
        Vector2 position = getRandomCell();
        while (isInDeque(snakeBody, position)) {
            position = getRandomCell();
        }
        return position;
    }
};

class Game {
private:
	Sound eatSound{};
	Sound wallSound{};
    double const speedFactor{0.8};
    int const speedInterval{ 5 };
   

public:
    Snake snake = Snake();
    Food food = Food("src/graphics/food.png", snake.body);
    int score{};
    double updateInterval{ 0.2 };
    bool isGameOver = false;
    bool isDirectionUpdated{ false };

    Game() {
        InitAudioDevice();
        eatSound = LoadSound("src/sounds/eat.mp3");
        wallSound = LoadSound("src/sounds/wall.mp3");
    }

    ~Game() {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    void draw() {
        snake.draw();
        food.draw();
    }

    void update() {
        if (!isGameOver) {
			snake.update();
			checkFoodCollision();
			checkEdgeCollison();
            checkCollisionWithBody();
            isDirectionUpdated = false;
        }
    }

    void checkFoodCollision() {
        if (Vector2Equals(snake.body[0], food.position)) {
            food.position = food.getRandomPos(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);

			// Increase speed every few points
			if (score % speedInterval == 0) {
                updateInterval = min(16.67, updateInterval * speedFactor);
			}
        }
    }

    void checkEdgeCollison() {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1) {
            GameOver();
        }

		if (snake.body[0].y == cellCount || snake.body[0].y == -1) {
			GameOver();
		}
    }

	void checkCollisionWithBody() {

		for (int i = 1; i < snake.body.size(); ++i) {
			// body[0] is head
			if (Vector2Equals(snake.body[0], snake.body[i])) {
                return GameOver();
			}
		}
	}

    void GameOver() {
        snake.reset();
        food.getRandomPos(snake.body);
        isGameOver = true;
        score = 0;
        updateInterval = 0.2;
        PlaySound(wallSound);
    }
};

int main() {
    InitWindow(2*offset + cellSize * cellCount, 2*offset + cellSize * cellCount, "Snek");
    SetTargetFPS(60);

    Game game = Game();

    while (!WindowShouldClose()) {
        BeginDrawing();

        if (isTimeElapsed(game.updateInterval)) {
            game.update();
        }

        if (!game.isDirectionUpdated) {
			if (IsKeyPressed(KEY_UP) && game.snake.dir.y != 1) {
				game.snake.dir = { 0, -1 };
				game.isGameOver = false;
                game.isDirectionUpdated = true;
			}
			if (IsKeyPressed(KEY_DOWN) && game.snake.dir.y != -1) {
				game.snake.dir = { 0, 1 };
				game.isGameOver = false;
                game.isDirectionUpdated = true;
			}
			if (IsKeyPressed(KEY_LEFT) && game.snake.dir.x != 1) {
				game.snake.dir = { -1, 0 };
				game.isGameOver = false;
                game.isDirectionUpdated = true;
			}
			if (IsKeyPressed(KEY_RIGHT) && game.snake.dir.x != -1) {
				game.snake.dir = { 1, 0 };
				game.isGameOver = false;
                game.isDirectionUpdated = true;
			}
        }
       

        ClearBackground(green);

        // draw grids
        for (int i = 0; i <= cellCount; ++i) {
            DrawLine(offset, i * cellSize + offset, cellSize * cellCount + offset, i * cellSize + offset, BLACK);

            DrawLine(i * cellSize + offset, offset, i * cellSize + offset, cellSize * cellCount + offset, BLACK);
        }
        

        DrawRectangleLinesEx(
            Rectangle{ (float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10,  (float)cellSize * cellCount + 10 }, 
            5, 
            darkGreen
        );
        DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
        DrawText(TextFormat("%i", game.score), offset + cellSize * cellCount - 10, 20, 40, WHITE);

        if (game.isGameOver) {
            DrawText("Press any arrow key to reset", offset - 5, 2 * offset + cellSize * cellCount - 30, 20, WHITE);
        }

        game.draw();
       
        EndDrawing();
    }

    CloseWindow();

    return 0;
}