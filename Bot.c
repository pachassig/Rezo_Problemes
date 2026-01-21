#include "Bot.h"

#include "map.h"

struct Bot* CreateBot()
{
    struct Bot* bot = (struct Bot*)malloc(sizeof(struct Bot));
    
    bot->position = (sfVector2i){0, 0};
    
    bot->sprite = sfSprite_create();
    sfTexture* tex = sfTexture_createFromFile("./Assets/Characters/Bot01.png", NULL);
    sfSprite_setTexture(bot->sprite, tex, sfTrue);
    sfSprite_setPosition(bot->sprite, (sfVector2f){0, 0});
    float scale = ((float)CELL_SIZE / 24.f) * 0.75f;
    sfSprite_setScale(bot->sprite, (sfVector2f){scale, scale});
    for (int i = 0; i < 5000; i++) {
        bot->MoveQueue[i].type = INVALID;
    }
    
    return bot;
}

void SpawnBotAtStartCell(struct Bot* bot, Grid* grid)
{
    for (int i = 0; i < GRID_ROWS; i++)
    {
        for (int j = 0; j < GRID_COLS; j++)
        {
            if (grid->cell[i][j]->type == START)
            {
                bot->position = grid->cell[i][j]->coord;
                sfVector2f startCelPosition = sfSprite_getPosition(grid->cell[i][j]->sprite);
                startCelPosition.x += 5.f;
                startCelPosition.y += 5.f;
                sfSprite_setPosition(bot->sprite, startCelPosition);
                break;
            }
        }
    }
    
}

void DestroyBot(struct Bot* bot)
{
    if (!bot->sprite) return;
    sfSprite_destroy(bot->sprite);
    free(bot);
}

void DrawBot(sfRenderWindow* window, struct Bot* bot)
{
    if (!window || !bot || !bot->sprite) return;
    sfRenderWindow_drawSprite(window, bot->sprite, NULL);
}

int MoveBot(struct Bot* bot, Grid* grid, enum MovementType type, enum Direction direction)
{
    int distance = 1;
    if (type == JUMP) distance = 2;
    
    sfVector2i newPosition = bot->position;
    
    switch (direction)
    {
    case NORTH:
        if (newPosition.y > 0)
        {
            newPosition.y -= distance;
        }
        break;
    case EAST:
        if (newPosition.x < (GRID_COLS - 1))
        {
            newPosition.x += distance;
        }
        break;
    case SOUTH:
        if (newPosition.y < (GRID_ROWS - 1))
        {
            newPosition.y += distance;
        }
        break;
    case WEST:
        if (newPosition.x > 0)
        {
            newPosition.x -= distance;
        }
        break;
    default:
        break;
    }

    enum CellType destinationCellType = grid->cell[newPosition.y][newPosition.x]->type;

    if (destinationCellType != OBSTACLE)
    {
        bot->position = newPosition;
        sfVector2f newSpritePosition = sfSprite_getPosition(grid->cell[bot->position.y][bot->position.x]->sprite);
        newSpritePosition.x += 5.f;
        newSpritePosition.y += 5.f;
        sfSprite_setPosition(bot->sprite, newSpritePosition);
    } else
    {
        printf("can't go there ! \n");
    }

    switch (destinationCellType)
    {
    case END:
        return REACH_END;
    case EMPTY:
        return FAILURE;
    case START:
    case WALKABLE:
    case OBSTACLE:
    default:
        return NOTHING;
    }
}

void AddMovement(struct Bot* bot, enum MovementType type, enum Direction direction)
{
    if (!bot) return;
    // Add a new element Move to bot's MoveQueue
    int currentLength = 0;
    while (bot->MoveQueue[currentLength].type == MOVE_TO || bot->MoveQueue[currentLength].type == JUMP)
    {
        currentLength++;
    }
    bot->MoveQueue[currentLength].type = type;
    bot->MoveQueue[currentLength].direction = direction;
    bot->MoveQueue[currentLength + 1].type = INVALID;
}

void MoveBot_AI(void* userData)
{
    struct GameData* data = (struct GameData*)userData;

    data->pathResult = NOTHING;

    while (1)
    {
        // ATTENDRE que le main applique le mouvement précédent
        while (data->hasMove) {
            sfSleep(sfMilliseconds(1));
        }

        // Vérifier qu'il reste un mouvement
        if (data->bot->MoveQueue[data->step].type == INVALID)
        {
            data->pathResult = NO_MOVE_LEFT;
            return;
        }

        // Préparer le prochain mouvement
        data->nextMove = data->bot->MoveQueue[data->step];
        data->hasMove = true;

        data->step++;

        
    }
}


bool SearchPath_AI(struct Bot* bot, Grid* grid)
{
    for (int i = 0; i < 5000; i++) {
        bot->MoveQueue[i].type = INVALID;
    }

    if (!bot || !grid) return false;

    int step = 0;
    sfVector2i current = bot->position;
    sfVector2i end = { -1, -1 };

    // Trouver la case END
    for (int y = 0; y < 20; y++)
    {
        for (int x = 0; x < 20; x++)
        {
            if (grid->cell[y][x]->type == END)
            {
                end.x = x;
                end.y = y;
                break;
            }
        }
        if (end.x != -1) break;
    }

    if (end.x == -1) return false;

    while ((current.x != end.x || current.y != end.y) && step < 4999)
    {
        int x = current.x;
        int y = current.y;
        /*END*/
        //Nord
        if (y - 1 >= 0 && (grid->cell[y - 1][x]->type == END) && bot->MoveQueue[step - 1].direction != SOUTH)
        {
            bot->MoveQueue[step].type = MOVE_TO;
            bot->MoveQueue[step].direction = NORTH;
            current.y -= 1;
            step++;
        }
        //Sud
        else if (y + 1 < 20 && (grid->cell[y + 1][x]->type == END) && bot->MoveQueue[step - 1].direction != NORTH)
        {
            bot->MoveQueue[step].type = MOVE_TO;
            bot->MoveQueue[step].direction = SOUTH;
            current.y += 1;
            step++;
        }
        //Est
        else if (x + 1 < 20 && (grid->cell[y][x + 1]->type == END) && bot->MoveQueue[step - 1].direction != WEST)
        {
            bot->MoveQueue[step].type = MOVE_TO;
            bot->MoveQueue[step].direction = EAST;
            current.x += 1;
            step++;
        }
        //Ouest
        else if (x - 1 >= 0 && (grid->cell[y][x - 1]->type == END) && bot->MoveQueue[step - 1].direction != EAST)
        {
            bot->MoveQueue[step].type = MOVE_TO;
            bot->MoveQueue[step].direction = WEST;
            current.x -= 1;
            step++;
        }

        /*WALKABLE*/
        // Nord
         else if (y - 1 >= 0 && (grid->cell[y - 1][x]->type == WALKABLE || grid->cell[y - 1][x]->type == END) && bot->MoveQueue[step - 1].direction != SOUTH)
        {
            bot->MoveQueue[step].type = MOVE_TO;
            bot->MoveQueue[step].direction = NORTH;
            current.y -= 1;
            step++;
        }
        else if (y - 2 >= 0 &&
            grid->cell[y - 1][x]->type != EMPTY &&
            (grid->cell[y - 2][x]->type == WALKABLE || grid->cell[y - 2][x]->type == END) && bot->MoveQueue[step - 1].direction != SOUTH)
        {
            bot->MoveQueue[step].type = JUMP;
            bot->MoveQueue[step].direction = NORTH;
            current.y -= 2;
            step++;
        }
        // Sud
        else if (y + 1 < 20 && (grid->cell[y + 1][x]->type == WALKABLE || grid->cell[y + 1][x]->type == END) && bot->MoveQueue[step-1].direction!=NORTH)
        {
            bot->MoveQueue[step].type = MOVE_TO;
            bot->MoveQueue[step].direction = SOUTH;
            current.y += 1;
            step++;
        }
        else if (y + 2 < 20 &&
            grid->cell[y + 1][x]->type != EMPTY &&
            (grid->cell[y + 2][x]->type == WALKABLE || grid->cell[y + 2][x]->type == END) && bot->MoveQueue[step - 1].direction != NORTH)
        {
            bot->MoveQueue[step].type = JUMP;
            bot->MoveQueue[step].direction = SOUTH;
            current.y += 2;
            step++;
        }
        // Est
        else if (x + 1 < 20 && (grid->cell[y][x + 1]->type == WALKABLE || grid->cell[y][x + 1]->type == END) && bot->MoveQueue[step - 1].direction != WEST)
        {
            bot->MoveQueue[step].type = MOVE_TO;
            bot->MoveQueue[step].direction = EAST;
            current.x += 1;
            step++;
        }
        else if (x + 2 < 20 &&
            grid->cell[y][x + 1]->type != EMPTY &&
            (grid->cell[y][x + 2]->type == WALKABLE || grid->cell[y][x + 2]->type == END) && bot->MoveQueue[step - 1].direction != WEST)
        {
            bot->MoveQueue[step].type = JUMP;
            bot->MoveQueue[step].direction = EAST;
            current.x += 2;
            step++;
        }
        // Ouest
        else if (x - 1 >= 0 && (grid->cell[y][x - 1]->type == WALKABLE || grid->cell[y][x - 1]->type == END) && bot->MoveQueue[step - 1].direction != EAST)
        {
            bot->MoveQueue[step].type = MOVE_TO;
            bot->MoveQueue[step].direction = WEST;
            current.x -= 1;
            step++;
        }
        else if (x - 2 >= 0 &&
            grid->cell[y][x - 1]->type != EMPTY &&
            (grid->cell[y][x - 2]->type == WALKABLE || grid->cell[y][x - 2]->type == END) && bot->MoveQueue[step - 1].direction != EAST)
        {
            bot->MoveQueue[step].type = JUMP;
            bot->MoveQueue[step].direction = WEST;
            current.x -= 2;
            step++;
        }
        
        else
        {
            return false; // plus de mouvements possibles
        }
    }

    // Marquer la fin de MoveQueue
    bot->MoveQueue[step].type = INVALID;
    return true;
}


