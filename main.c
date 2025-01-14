#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#define SCALE_FACTOR 1.6
#define SCREEN_HEIGHT 450
#define SCREEN_WIDTH 800
#define LAYER_COUNT 2
#define MAX_LIVES 3
#define GRAVITY 15.0f
#define MAX_ENEMIES 12
#define MAX_ZOMBIE_HANDS 15
#define MAX_POTIONS 3
#define NUM_POTIONS 3

typedef enum { IDLE, RUNNING, JUMPING, ATTACK, HURT, DEAD } PersonagemState;
typedef enum { START_SCREEN, GAMEPLAY, FINAL } GameState;
const char *historiaDoJogo = "Em uma sociedade marcada pela decadência, a elite recrutou uma\n\n" 
                               "\tequipe de cientistas, e após anos de pesquisa em um projeto\n\n"
                                "\t\tsecreto, criou-se uma substância destinada à imortalidade,\n\n"
                                "\t\t acreditando ser a única esperança para a sobrevivência\n\n"
                                "\t\t\t\t\t\thumana. Porém, o experimento saiu do controle,\n\n"
                                "\t\t\t\t\t transformando a maioria da população em zumbis.\n\n "
                                "\nVocê é um dos poucos que escaparam de uma tentativa de invasão\n\n"
                                "à essa fortaleza, mas agora, na floresta densa, hordas de zumbis \n\n"
                                "\testão por toda parte. Para alcançar a segurança, você deve\n\n"
                                "\t\tcorrer por perigosas áreas infestadas e coletar suprimentos.\n\n"
                                "\t\tSua meta é chegar à um abrigo subterrâneo, onde os últimos\n\n"
                                "\t\t\t\t\t\t\t\t\t\t\tcientistas tentam criar uma vacina.\n\n";
const char *tituloDoJogo = "Pitfall: Rise of Dead";

// struct player
typedef struct{
    int x;
    int y;
    int width;
    int height;
    float velocityY; // velocidade vertical (para controlar o pulo)
    bool isJumping;
    bool canJump;
    PersonagemState state;
    Texture2D idleTexture;
    Texture2D deadTexture;
    Texture2D runTexture;
    Texture2D jumpTexture;
    Texture2D attackTexture;
    int frame;           // Frame atual da animação
    float frameTime;     // Tempo entre frames
    float currentFrameTime; // Acumulador de tempo
    int maxFrames;       // Número de frames na spritesheet
    bool flipRight; // controla a direção
    int lives;
    Texture2D heartTexture3;
    Texture2D heartTexture2;
    Texture2D heartTexture1;
    bool invencivel; //tempo de invencibilidade logo apos levar uma colisao
    float invencibilidadeTimer; // tempo que dura a invencibilidade
    bool isAttacking;
    bool isDead;
} Player;


// struct enemy
typedef struct{
    int x;
    int y;
    int width;
    int height;
    float velocityY;
    bool isAttacking;
    PersonagemState state;
    Texture2D idleTexture;
    Texture2D runTexture;
    Texture2D attackTexture;
    int frame;           // Frame atual da animação
    float frameTime;     // Tempo entre frames
    float currentFrameTime; // Acumulador de tempo
    int maxFrames;       // Número de frames na spritesheet
    bool flipRight; // controla a direção
    int enemyLives;
    Texture2D hurtTexture;
    Texture2D deadTexture;
    bool isDead;
    bool decreaseLives;
    int lives;
    bool invencivel; //tempo de invencibilidade logo apos levar uma colisao
    float invencibilidadeTimer; // tempo que dura a invencibilidade
} Enemy;

typedef struct {
    Enemy enemy;
    bool isActive;
    float spawnX;
} EnemySpawner;

typedef struct {
    Texture2D texture;
    Vector2 position;
    float speed;
} BackgroundLayer;

typedef enum {
    PIT,
    FLOOR,
} PlatformType;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    PlatformType type;
    Texture2D texture;
} Platform;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int initialX;
    Texture2D texture;
    bool isActive;
} ZombieHand;
ZombieHand zombie_hands[MAX_ZOMBIE_HANDS];

typedef struct {
    int x;
    int y;
    int width;
    int height;
    Texture2D texture;
    bool active;
} Potion;

Potion potions[MAX_POTIONS]; // Array para as poções
int activePotions = 0; 

struct Winners{
    char nome[20];
    int tempo;
    struct Winners *next;
};
struct Winners *head = NULL;

void add_winner(struct Winners **head, char *nome, int tempo) {
    struct Winners *n = *head;
    struct Winners *novo = (struct Winners *)malloc(sizeof(struct Winners));
    struct Winners *anterior = NULL;

    if (novo == NULL) {
        perror("Falha ao alocar memória");
        exit(1);
    }

    strcpy(novo->nome, nome);
    novo->tempo = tempo;
    novo->next = NULL;

    if (*head == NULL) { // Caso a lista esteja vazia
        *head = novo;
        return;
    }

    if ((*head)->tempo > novo->tempo) { // Inserir no início da lista
        novo->next = *head;
        *head = novo;
        return;
    }

    // Percorre a lista até encontrar a posição correta
    while (n != NULL && n->tempo <= novo->tempo) {
        anterior = n;
        n = n->next;
    }

    if (anterior != NULL) {
        anterior->next = novo;
    }
    novo->next = n;
}

void winnerList(){
    FILE *list;
    char nome[20];
    int tempo;
    list = fopen("vencedores.txt", "r");
    while(fscanf(list, "%s %d", nome, &tempo) == 2){
        add_winner(&head, nome, tempo);
    }
    fclose(list);
}

void writeWinners(){
    FILE *list;
    list = fopen("winners.txt", "a");
    struct Winners *n = head;
    while(n != NULL){
        fprintf(list,"%s %d\n", n->nome, n->tempo);
        n=n->next;
    }
    fclose(list);
}

void DrawTimer(Camera2D camera, int timerValue);
void DrawLives(Player player, Camera2D camera);
//player
void UpdatePlayerAnimation(Player *player, float deltaTime);
void DrawPlayer(Player player);
void UpdateEnemyAnimation(Enemy *enemy, float deltaTime);
//enemy
void DrawEnemy(Enemy enemy);
void InitEnemySpawners(EnemySpawner enemies[], int count, Enemy baseEnemy);
void UpdateEnemies(EnemySpawner enemies[], int count, Player player, Platform *platforms, int total_ground_count);
void InitZombieHands(ZombieHand hands[], int count, int screenWidth, int screenHeight, Texture2D zombiehand_texture, int platform_height);
void UpdateEnemyPosition(Enemy *enemy, Player player);
//gravidade e colisão
int player_na_plataforma(Player player, Platform platforms[], int total_ground_count);
int enemy_na_plataforma(Enemy enemy, Platform platforms[], int total_ground_count);
bool enemy_colide_player(Enemy enemy, Player player);
bool player_colide_enemy(Enemy enemy, Player player);
void aplica_gravidade_player(Player *player, Platform platforms[], int total_ground_count, float deltaTime);
void aplica_gravidade_enemy(Enemy *enemy, Platform platforms[], int total_ground_count, float deltaTime);
//hands
void DrawZombieHands(ZombieHand hands[], int count);
void UpdateZombieHands(ZombieHand hands[], int count, Player player, bool *colidiuHand);
bool player_colide_hand(ZombieHand hand, Player player);
//potion
bool player_colide_potion(Player player, Potion potion, int screenHeight, int platform_height);

void DrawBackground(Texture2D background, int screenWidth, int screenHeight, Camera2D camera) {
    float scale;
    float deslocamento_x = 0;
    float deslocamento_y = 0;

    // calcula fator de escala p cobrir a tela inteira
    float scaleX = (float)screenWidth / background.width;
    float scaleY = (float)screenHeight / background.height;
    
    // escolhe o maior fator de escala p garantir que a imagem cubra a tela
    if (scaleX > scaleY) {
        scale = scaleX;
    } else {
        scale = scaleY;
    }

    // ajusta deslocamento para centralizar a imagem
    if (background.width * scale > screenWidth)
        deslocamento_x = (screenWidth - (background.width * scale)) / 2;
    if (background.height * scale > screenHeight)
        deslocamento_y = (screenHeight - (background.height * scale)) / 2;

    // aplica o deslocamento da câmera para o efeito parallax
    deslocamento_x += camera.offset.x * 0.1f; 
    deslocamento_y += camera.offset.y * 0.1f;

    DrawTextureEx(background, (Vector2){deslocamento_x, deslocamento_y}, 0, scale, WHITE);
}

void DrawLives(Player player, Camera2D camera) {
    // Define a posição inicial para desenhar as vidas, ajustada pela posição da câmera
    int posX = 20 - camera.offset.x;
    int posY = 20 - camera.offset.y;

    // Desenha o ícone de vida correto baseado na quantidade de vidas do jogador
    if (player.lives == 3) {
        DrawTexture(player.heartTexture3, posX, posY, WHITE);
    } else if (player.lives == 2) {
        DrawTexture(player.heartTexture2, posX, posY, WHITE);
    } else if (player.lives == 1) {
        DrawTexture(player.heartTexture1, posX, posY, WHITE);
    }
}
void DrawTimer(Camera2D camera, int timerValue) {
    // Define a posição do timer ajustada pela câmera
    int posX = 20 - camera.offset.x;
    int posY = 20 - camera.offset.y;

    // Converte o valor do timer para string
    char timerText[10];
    snprintf(timerText, sizeof(timerText), "%d", timerValue);

    DrawText(timerText, posX, posY, 20, RED);
}

void UpdatePlayerAnimation(Player *player, float deltaTime) {
    player->currentFrameTime += deltaTime;

    // avança para o próximo frame se o tempo decorrido for suficiente
    if (player->currentFrameTime >= player->frameTime) {
        player->currentFrameTime = 0;
        player->frame++;
        
        // volta para o primeiro frame ao atingir o último
        if (player->frame >= player->maxFrames) {
            player->frame = 0;
        }
    }
}

void DrawPlayer(Player player) {
    Texture2D texture;
    
    switch (player.state) {
        case RUNNING:
            texture = player.runTexture;
            break;
        case JUMPING:
            texture = player.jumpTexture;
            break;
        case ATTACK:
            texture = player.attackTexture;
            break;
        case DEAD:
            texture = player.deadTexture;
            break;
        case IDLE:
        default:
            texture = player.idleTexture;
            break;
    }
    
    if (player.maxFrames <= 0) {
        player.maxFrames = 1;  // safe default
    }
    int frameWidth = texture.width / player.maxFrames;
    
    // define a área do frame atual na spritesheet
    Rectangle sourceRect = { frameWidth * player.frame, 0, frameWidth, texture.height };

    // inverte o `sourceRect.width` se `flipRight` for false
    if (!player.flipRight) {
        sourceRect.width = -frameWidth;
    }

    Rectangle destRect = { player.x, player.y, player.width * 6, player.height * 6 };
    Vector2 origin = { 0, 0 };
    
    // desenha o frame atual da spritesheet, aplicando o flip horizontal se necessário
    DrawTexturePro(texture, sourceRect, destRect, origin, 0.0f, WHITE);
}

void UpdateEnemyAnimation(Enemy *enemy, float deltaTime) {
    enemy->currentFrameTime += deltaTime;

    if (enemy->currentFrameTime >= enemy->frameTime) {
        enemy->currentFrameTime = 0;
        enemy->frame++;
        
        switch(enemy->state) {
            case RUNNING:
                if (enemy->frame >= 7) {
                    enemy->frame = 0;
                    enemy->frameTime = 0.5f;
                    break;
                }
                
            case ATTACK:
                if (enemy->frame >= 4) {
                    enemy->frame = 0;
                    enemy->state = IDLE;
                    enemy->maxFrames = 8;
                    enemy->frameTime = 1.0f;
                }
                break;
                
            case IDLE:
                if (enemy->frame >= 8) enemy->frame = 0;
                break;
            
            case HURT:
                if (enemy->frame < 3) { // avança os frames apenas até o último
                    enemy->frame++;
                }
                // quando atingir o ultimo frame, fica parado nele
                if (enemy->frame >= 3) {
                    enemy->frame = 3; // fixa no último frame
                }
                break;
            case DEAD:
                if (enemy->frame < 4) { // avança os frames apenas até o último
                    enemy->frame++;
                }
                if (enemy->frame >= 4) {
                    enemy->frame = 4; // fixa no último frame
                }
                break;
            default:
                if (enemy->frame >= enemy->maxFrames) enemy->frame = 0;
                break;
        }
    }
}

void DrawEnemy(Enemy enemy) {
    Texture2D texture;
    
    switch (enemy.state) {
        case RUNNING:
            texture = enemy.runTexture;
            break;
        case ATTACK:
            texture = enemy.attackTexture;
            break;
        case HURT:
            texture = enemy.hurtTexture;
            break;
        case DEAD:
            texture = enemy.deadTexture;
            break;
        case IDLE:
        default:
            texture = enemy.idleTexture;
            break;
    }

    if (enemy.maxFrames <= 0) {
        enemy.maxFrames = 1;  // safe default
    }
    int frameWidth = texture.width / enemy.maxFrames;
        
    // define a área do frame atual na spritesheet
    Rectangle sourceRect = { frameWidth * enemy.frame, 0, frameWidth, texture.height };

    // inverte o `sourceRect.width` se `flipRight` for false
    if (!enemy.flipRight) {
        sourceRect.width = -frameWidth;
    }

    Rectangle destRect = { enemy.x, enemy.y, enemy.width * 6, enemy.height * 6 };
    Vector2 origin = { 0, 0 };
    
    DrawTexturePro(texture, sourceRect, destRect, origin, 0.0f, WHITE);
}

void InitEnemySpawners(EnemySpawner enemies[], int count, Enemy baseEnemy) {
    float spawnDistance = 800.0f;
    
    for(int i = 0; i < count; i++) {
        enemies[i].enemy = baseEnemy;
        enemies[i].isActive = false;
        enemies[i].spawnX = (i + 1) * spawnDistance;
        enemies[i].enemy.decreaseLives = false;
    }
}

int player_na_plataforma(Player player, Platform platforms[], int total_ground_count) {
    Rectangle player_rec = {
        .x = player.x + (player.width * 2),
        .y = player.y + (player.height * 6) - 10,
        .width = player.width,
        .height = 10  // Small height for ground detection
    };

    for(int i = 0; i < total_ground_count; i++) {
        Rectangle platform_rec = {
            .x = platforms[i].x,
            .y = platforms[i].y,
            .width = platforms[i].width,
            .height = platforms[i].height
        };

        if(CheckCollisionRecs(player_rec, platform_rec)) {
            if (platforms[i].type == FLOOR){
                return i;
            }
        }
    }
    return -1;
}

int enemy_na_plataforma(Enemy enemy, Platform platforms[], int total_ground_count) {
    Rectangle enemy_rec = {
        .x = enemy.x + (enemy.width * 2),
        .y = enemy.y + (enemy.height * 6) - 10,
        .width = enemy.width * 2,
        .height = 10 
    };

    for(int i = 0; i < total_ground_count; i++) {
        Rectangle platform_rec = {
            .x = platforms[i].x,
            .y = platforms[i].y,
            .width = platforms[i].width,
            .height = platforms[i].height
        };

        if(CheckCollisionRecs(enemy_rec, platform_rec)) {
            if (platforms[i].type == FLOOR){
                return i;
            }
        } 
    }
    return -1;
}

bool enemy_colide_player(Enemy enemy, Player player){
    Rectangle enemy_rec = {
        .x = enemy.x + (enemy.width * 2),
        .y = enemy.y,
        .width = enemy.width * 3,
        .height = 10 
    };

    Rectangle player_rec = {
        .x = player.x + (player.width * 2),
        .y = player.y,
        .width = player.width * 3,
        .height = 10 
    };

    return CheckCollisionRecs(enemy_rec, player_rec);
    
}

bool player_colide_enemy(Enemy enemy, Player player){
    Rectangle player_rec = {
        .x = player.x + (player.width * 2),
        .y = player.y,
        .width = player.width * 3,
        .height = 10 
    };

    Rectangle enemy_rec = {
        .x = enemy.x + (enemy.width * 2),
        .y = enemy.y,
        .width = enemy.width * 3,
        .height = 10 
    };
    return CheckCollisionRecs(player_rec, enemy_rec);
}

void UpdateEnemyPosition(Enemy *enemy, Player player) {
    if(enemy -> isDead){
        if(enemy->state != DEAD){
            enemy->state = DEAD;
            enemy->frame = 0;
            enemy->maxFrames = 5;
            enemy->frameTime = 0.2f;
        }
        return;
    }
    bool is_colliding = enemy_colide_player(*enemy, player);

    if (is_colliding){
        if(enemy->state != HURT && player.isAttacking && enemy->state != DEAD){
            enemy->state = HURT;
            enemy->frame = 0;
            enemy->maxFrames = 3;
            enemy->frameTime = 0.2f;
        }
        enemy->isAttacking = true;
        if (enemy->state != ATTACK && !player.isAttacking && enemy->state != DEAD) {
            enemy->state = ATTACK;
            enemy->frame = 0;
            enemy->maxFrames = 4;
            enemy->frameTime = 0.2f;
        }
    }
    
    else if (player.x > enemy->x && !is_colliding){
        enemy->x += 2.0f;
        enemy->flipRight = true;
        enemy->isAttacking = false;
        if (enemy->state != RUNNING) {
            enemy->state = RUNNING;
            enemy->frame = 0;
            enemy->maxFrames = 7;
            enemy->frameTime = 0.3f;
        }
    } else if (player.x < enemy->x && !is_colliding){
        enemy->x -= 2.0f;
        enemy->flipRight = false;
        enemy->isAttacking = false;
        if (enemy->state != RUNNING) {
            enemy->state = RUNNING;
            enemy->frame = 0;
            enemy->maxFrames = 7;
            enemy->frameTime = 0.3f;
        }
    } 
    else {
        enemy->isAttacking = false;
        if (enemy->state != IDLE && enemy->state != DEAD) {
            enemy->state = IDLE;
            enemy->frame = 0;
            enemy->maxFrames = 8;
            enemy->frameTime = 0.3f;
        }
    }
}

void UpdateEnemies(EnemySpawner enemies[], int count, Player player, Platform *platforms, int total_ground_count) {
    float spawnDistance = 1000.0f; 

    for(int i = 0; i < count; i++) {
        if (!enemies[i].isActive) {
            // verifica se o player ta chegando no spawn
            if (player.x > (enemies[i].spawnX - spawnDistance)) {
                enemies[i].isActive = true;
                enemies[i].enemy.x = enemies[i].spawnX;
                enemies[i].enemy.y = player.y;
            }
        }
        else {
            aplica_gravidade_enemy(&enemies[i].enemy, platforms, total_ground_count, GetFrameTime());
            UpdateEnemyPosition(&enemies[i].enemy, player);
            UpdateEnemyAnimation(&enemies[i].enemy, GetFrameTime()); 
        }
    }
}

void aplica_gravidade_player(Player *player, Platform platforms[], int total_ground_count, float deltaTime) {

    const float MAX_FALL_SPEED = 10.0f;
    int current_platform = player_na_plataforma(*player, platforms, total_ground_count);
    bool on_floor = (current_platform != -1); // player ta no FLOOR
    
    // gravity se nao tiver no chão
    if (!on_floor || player->isJumping) {
        player->velocityY += GRAVITY * deltaTime;
    }

    // fall speed
    if (player->velocityY > MAX_FALL_SPEED) {
        player->velocityY = MAX_FALL_SPEED;
    }
    
    // Update position
    player->y += player->velocityY * deltaTime * 60.0f;
    
    // verifica colisao pós movimento
    current_platform = player_na_plataforma(*player, platforms, total_ground_count);

    if (current_platform != -1) {
        // if colliding floor
        if (player->velocityY > 0) {  // se tiver descendo
            player->isJumping = false;
            player->y = platforms[current_platform].y - (player->height * 6);
            player->velocityY = 0.0;
            player->canJump = true;    
        }   
    } else {
        if (player->velocityY != 0) {
            player->canJump = false;
        }
    }
}

void aplica_gravidade_enemy(Enemy *enemy, Platform platforms[], int total_ground_count, float deltaTime) {
    const float MAX_FALL_SPEED = 10.0f;  // max falling speed
    
    // gravity with deltaTime
    enemy->velocityY += GRAVITY * deltaTime;

    if (enemy->velocityY > MAX_FALL_SPEED) {
        enemy->velocityY = MAX_FALL_SPEED;
    }
    
    enemy->y += enemy->velocityY * deltaTime * 60.0f;
    
    int current_platform_enemy = enemy_na_plataforma(*enemy, platforms, total_ground_count);

    if (current_platform_enemy != -1) {
        if (enemy->velocityY > 0) {  // se tiver caindo
            enemy->y = platforms[current_platform_enemy].y - (enemy->height * 6);
            enemy->velocityY = 0;
        }
    }
}

void InitZombieHands(ZombieHand hands[], int count, int screenWidth, int screenHeight, Texture2D zombiehand_texture, int platform_height) {
    float spawnDistance = 760.0f;
  
    for(int i = 0; i < count; i++) {
        hands[i].x = (i + 1) * spawnDistance;
        hands[i].y = screenHeight - platform_height - hands[i].height - 30;  // bota no topo da plataform
        hands[i].width = 60;
        hands[i].height = 60;
        hands[i].texture = zombiehand_texture;
        hands[i].isActive = false;
        hands[i].initialX = hands[i].y + (hands[i].height * 6) - 10; // initial spawn position
    }
}

void UpdateZombieHands(ZombieHand hands[], int count, Player player, bool *colidiuHand) {
    float spawnDistance = 800.0f;
    *colidiuHand = false;
    
    for(int i = 0; i < count; i++) {
        if (!hands[i].isActive) {
            if (player.x > (hands[i].initialX - spawnDistance)) {
                hands[i].isActive = true;
            }
        }
        else if (hands[i].isActive) {
           if(player_colide_hand(hands[i], player)){
                *colidiuHand = true;
                break;
            }

        }
    }
}

bool player_colide_hand(ZombieHand hand, Player player){
    Rectangle hand_rec = {
        .x = hand.x + 22,
        .y = hand.y,
        .width = hand.width - 35,
        .height = hand.height * 2 
    };

    Rectangle player_rec = {
        .x = player.x + (player.width * 2) + 35,
        .y = player.y + (player.height * 6) - 10,
        .width = player.width - 15,
        .height = 10
    };
    
    return CheckCollisionRecs(hand_rec, player_rec);
}

void DrawZombieHands(ZombieHand hands[], int count) {
    for(int i = 0; i < count; i++) {
        
        // pula se a position é invalida OR inativa
        if (hands[i].x == -1 || hands[i].y == -1 || !hands[i].isActive) {
            continue;
        }

        Rectangle source = { 0, 0, hands[i].width, hands[i].height };
        Rectangle zombiehand = { 
            hands[i].x,
            hands[i].y,
            hands[i].width,
            hands[i].height 
        };
        
        DrawTexturePro(hands[i].texture, source, zombiehand, (Vector2){0, 0}, 0.0f, WHITE);
    }
}

bool player_colide_potion(Player player, Potion potion, int screenHeight, int platform_height){
    
    Rectangle potion_rec = {
        .width = potion.width,
        .height = potion.height, 
        .x = potion.x,
        .y = potion.y,

    };

    Rectangle player_rec = {
        .x = player.x + (player.width * 2) + 35,
        .y = player.y + (player.height * 2) + 60,
        .width = player.width - 15,
        .height = player.height * 3
    };
    
    return CheckCollisionRecs(potion_rec, player_rec);
}


int main(void){
    bool isFinalPhaseTriggered = false;
    bool isGameOver = false;
    int screenWidth = SCREEN_WIDTH * SCALE_FACTOR;
    int screenHeight = SCREEN_HEIGHT * SCALE_FACTOR;
    int potionsCollected = 0;
    static char nome[256]= ""; // buffer pro texto
    int letterCount = 0; // n caracteres no texto
    int maxLength = 30; // limite de caracteres
    int foiAdicionado = 0;

    InitWindow(screenWidth, screenHeight, "Pitfall - Rise Of Dead");
    InitAudioDevice();
    double startTime = 0.0;
    double elapsedTime = 0;
    bool timeStarted = false;
    Texture2D backgroundTitle = LoadTexture("assets/map/layers/initial-bg.png");
    Texture2D floor_texture = LoadTexture("assets/map/floor.png");
    Texture2D pit2_texture = LoadTexture("assets/obstaculos/a.png");
    Texture2D background_texture = LoadTexture( "assets/map/layers/bg1.png" );
    Texture2D background2_texture = LoadTexture( "assets/map/layers/bg2.png" );
    Texture2D finalfloor_texture = LoadTexture( "assets/map/buraco-fim.png" );
    Texture2D potionTextures[NUM_POTIONS];
    potionTextures[0] = LoadTexture("assets/potions/potion-gold-solo.png");    
    potionTextures[1] = LoadTexture("assets/potions/potion-red-solo.png");  
    potionTextures[2] = LoadTexture("assets/potions/potion-purple-solo.png");  

    Texture2D PotionIcon = LoadTexture("assets/potions/potion-gold-solo.png");

    Music music = LoadMusicStream("assets/sounds/thriller.wav");
    Texture2D zombiehand_texture = LoadTexture( "assets/obstaculos/zombiehand.png" );

    SetTargetFPS(60);
    GameState gameState = START_SCREEN;

    Vector2 camera_deslocamento = {
        .x = 0,
        .y = 0,
    };

    Vector2 camera_alvo = {
        .x = 0,
        .y = 0,
    };

    Camera2D camera = {
        .offset = camera_deslocamento,
        .target = camera_alvo,
        .rotation = 0,
        .zoom = 1,
    };

    int worldWidth = screenWidth * 10;

    int background_width = 1820;
    int num_backgrounds_needed = (worldWidth / background_width) + 1;

    // definicões da plataforma
    int whitespace = 30; // espaco em branco da imagem, essa "margem/padding" do topo

    int platform_height = 190; // altura do chão
    int platform_width = 200; // tamanho (largura) de cada plataforma

    int total_ground_count = ceil((float)worldWidth / (float)platform_width); // calcula quantos pedacos de chão são necessários pra cobrir toda a largura do mundo
    
    Platform platforms[total_ground_count];
    
    int platform_x = 0;  // acumula 

    for(int i=0; i < total_ground_count; i++){
        platforms[i].width = platform_width;
        platforms[i].height = platform_height;
        platforms[i].y = screenHeight - platform_height + whitespace;
        platforms[i].x = platform_x;
        if(i%7 == 6){ 
            platforms[i].type = PIT; // Buraco
        } else {
            platforms[i].type = FLOOR; // Chão
        }
        platform_x += platform_width;
    }

    InitZombieHands(zombie_hands, MAX_ZOMBIE_HANDS, screenWidth, screenHeight, zombiehand_texture, platform_height);
    bool colidiuHand = false;

    // cria player
    Player player = {
        .x = SCREEN_WIDTH / 2,
        .y = 175,
        .width = 64,
        .height = 64,
        .state = IDLE,
        .isJumping = false,
        .canJump = true,
        .idleTexture = LoadTexture("assets/player/idle2.png"),
        .deadTexture = LoadTexture("assets/player/Dead.png"),
        .runTexture = LoadTexture("assets/player/run.png"),
        .jumpTexture = LoadTexture("assets/player/jump.png"),
        .attackTexture = LoadTexture("assets/player/attack.png"),
        .maxFrames = 5, // quantidade de frames de IDLE
        .frame = 0,
        .frameTime = 0.3f, // Tempo entre frames
        .currentFrameTime = 0.0f,
        .flipRight = true, 
        .velocityY = 0,  
        .lives = MAX_LIVES,  
        .heartTexture3 = LoadTexture("assets/Heart/Heart3.png"),
        .heartTexture2 = LoadTexture("assets/Heart/Heart2.png"),
        .heartTexture1 = LoadTexture("assets/Heart/Heart1.png"),
        .isDead = false,
    };
    
    // cria enemy
    Enemy enemy = {
        .x = SCREEN_WIDTH / 2,
        .y = 150,
        .width = 64,
        .height = 64,
        .state = IDLE,
        .isAttacking = false,
        .idleTexture = LoadTexture("assets/zombie/Idle.png"),
        .runTexture = LoadTexture("assets/zombie/run.png"),
        .attackTexture = LoadTexture("assets/zombie/attack.png"),
        .maxFrames = 8,
        .frame = 0,
        .frameTime = 0.3f,
        .currentFrameTime = 0.0f,
        .flipRight = true,
        .velocityY = 0,
        .hurtTexture = LoadTexture("assets/zombie/Hurt.png"),
        .deadTexture = LoadTexture("assets/zombie/Dead.png"),
        .lives = MAX_LIVES,
    };

    EnemySpawner enemies[MAX_ENEMIES];
    InitEnemySpawners(enemies, MAX_ENEMIES, enemy);

    // Inicialização das poções
    srand(time(NULL));
    int spawnDistance = 4000;
    for (int i = 0; i < MAX_POTIONS; i++) {
        potions[i].width = potions[i].width - 35,
        potions[i].height = potions[i].height * 2,
        potions[i].x = (i + 1) * spawnDistance,  // x position with offset
        potions[i].y = screenHeight - platform_height - potions[i].height - 30, // y position with offset
        potions[i].texture = potionTextures[i];
        potions[i].active = true;
        activePotions++;
    }
    
    // game loop
    while (!WindowShouldClose()){
        bool colidiu = false;
        bool movingHorizontal = false;
        bool movingLeft = false;
        float deltaTime = GetFrameTime();

        // draw the game
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if(gameState == START_SCREEN){
            BeginDrawing();
            if(IsKeyPressed(KEY_ENTER)){
                gameState = GAMEPLAY;
                startTime = GetTime(); //momento do começo do jogo
                timeStarted = true;
                isGameOver = false;
            }
            int posXtitulo = 420;
            int postYtitulo = 40;
            int posXhistoria = 155;
            int posYhistoria = 150;
            int fonttitulo = 40;
            int fonthistoria = 30;
            Vector2 posTitulo = { 420, 40 };
            Vector2 posHistoria = { 155, 150 };

            DrawBackground(backgroundTitle, screenWidth, screenHeight, camera);
            
            DrawText(tituloDoJogo, 480, 40, 30, GREEN); //posicao X, posicao Y, tamanho fonte, cor
            DrawText(historiaDoJogo, 300, 150, 20, LIGHTGRAY);
            DrawText("Pressione ENTER para iniciar a corrida!", 390, 540, 25, DARKGREEN);
        }
        else if(gameState == GAMEPLAY){
            PlayMusicStream(music);
            UpdateMusicStream(music);
            if (!timeStarted) {
                startTime = GetTime(); // Garante que o tempo de início é capturado apenas uma vez
                timeStarted = true;
            }
            if (!isGameOver) {
                double currentTime = GetTime();
                elapsedTime = currentTime - startTime; // Atualiza o tempo decorrido
            }
            // camera 2D
            BeginMode2D( camera );
            if( player.x > screenWidth * 0.1 ) {
                camera.offset.x = -(player.x - screenWidth * 0.1);

            } else if( player.x < screenWidth * 0.05 ) {
                camera.offset.x = -(player.x - screenWidth * 0.05);
            }

            if( camera.offset.x > 0 ) {
                camera.offset.x = 0;
            }

            if( camera.offset.x < -(worldWidth - screenWidth) ) {
                camera.offset.x = -(worldWidth - screenWidth);
            }

            const float MOVE_SPEED = 5.0f; // constante velocidade player

            // movimento player
            if (IsKeyPressed(KEY_W) && !player.isJumping) {
                player.velocityY = -400.0f * GetFrameTime();
                player.isJumping = true;
                player.canJump = false;
                player.state = JUMPING;
                player.frame = 0;
                player.maxFrames = 8;
                player.frameTime = 0.2f;
                player.isAttacking = false;
            }
            
            // Handle horizontal movement
            if (IsKeyDown(KEY_A)) {
                player.x -= MOVE_SPEED;
                movingHorizontal = true;
                movingLeft = true;
                player.flipRight = false;
                player.isJumping = false;
                player.isAttacking = false;

                if (player.state != RUNNING && !player.isDead) {
                    player.state = RUNNING;
                    player.frame = 0; // Reseta o frame ao entrar no estado RUNNING
                    player.maxFrames = 8;
                    player.frameTime = 0.1f;
                }
                if(player.y < 176 || player.y > 176){
                    player.isJumping = true;
                }
            }

            if (IsKeyDown(KEY_D)) {
                player.x += MOVE_SPEED;
                movingHorizontal = true;
                movingLeft = false;
                player.flipRight = true;
                player.isJumping = false;
                player.isAttacking = false;
                
                if (player.state != RUNNING && !player.isDead) {
                    player.state = RUNNING;
                    player.frame = 0;
                    player.maxFrames = 8;
                    player.frameTime = 0.1f;
                }
                if(player.y < 176 || player.y > 176 && !player.isDead){
                    player.isJumping = true;
                }
            }
            if (IsKeyDown(KEY_R)) {
                if (player.state != ATTACK && !player.isJumping && !player.isDead) {
                    player.state = ATTACK;
                    player.frame = 0;
                    player.maxFrames = 5;
                    player.frameTime = 0.1f;
                    player.isAttacking = true;
                }
            }
            else if (!movingHorizontal && !player.isJumping && !player.isDead) {
                player.isAttacking = false;
                if (player.state != IDLE) {
                    player.state = IDLE;
                    player.frame = 0;
                    player.maxFrames = 5;
                    player.frameTime = 0.3f;
                }
            } if(player.isDead){
                if(player.state != DEAD){
                    player.state = DEAD;
                    player.frame = 0;
                    player.maxFrames = 4;
                    player.frameTime = 0.2f;
                }
            }

            aplica_gravidade_player(&player, platforms, total_ground_count, deltaTime);
            
            if(player.y > 180){
                player.lives = 0;
                isGameOver = true;
                player.isDead = true;
            }

            // limites do player
            if (player.x < 0) {
                player.x = 0;  // para na esquerda
            } else if (player.x > (worldWidth - (player.width * 3))) {
                player.x = (worldWidth - (player.width * 3));  // para na direita
            }

            UpdatePlayerAnimation(&player, deltaTime);

            // primeiro background layer (moves slower)
            for (int i = 0; i < num_backgrounds_needed; i++) {
                DrawTexture(background_texture, i * background_width - (camera.target.x * 0.3f), 0, WHITE);
            } // (0.3 = 30% da camera speed)

            // segundo background layer (moves faster)
            float parallax_factor = 0.3f;  
            for (int i = 0; i < num_backgrounds_needed; i++) {
                DrawTexture(background2_texture, i * background_width - camera.target.x, 0, WHITE);
            }
            
            UpdateEnemies(enemies, MAX_ENEMIES, player, platforms, total_ground_count);
            UpdateZombieHands(zombie_hands, MAX_ZOMBIE_HANDS, player, &colidiuHand);
            DrawPlayer(player);

            for(int i = 0; i < MAX_ENEMIES; i++) {
                if(enemies[i].isActive) {
                    DrawEnemy(enemies[i].enemy);
                }
            }

            for (int i = 0; i < MAX_POTIONS; i++) {
                if (potions[i].active) {
                    if (player_colide_potion(player, potions[i], screenHeight, platform_height)) {
                        potions[i].active = false;
                        activePotions--;
                        potionsCollected++;
                        
                    }
                    DrawTexture(potions[i].texture, potions[i].x, potions[i].y, WHITE);
                }
            }

            int posX = (screenWidth - 60) - camera.offset.x;
            int posY = 30 - camera.offset.y;
            DrawTexture(PotionIcon, posX-30, posY, WHITE);
            DrawText(TextFormat("%d/3", potionsCollected), posX, posY+10, 20, WHITE);

            //desenhar floor/pit
            for (int i = 0; i < total_ground_count; i++) {
                Texture2D platform_texture;
                
                // alterna entre as texturas conforme o tipo da platform
                if (platforms[i].type == FLOOR) {
                    DrawZombieHands(zombie_hands, MAX_ZOMBIE_HANDS);
                    platform_texture = floor_texture;  // usa `floor_texture` para FLOOR
                } else {
                    platform_texture = pit2_texture;  // usa `pit2_texture` para PIT
                }

                DrawTexture(platform_texture, platforms[i].x, platforms[i].y - whitespace, WHITE);
            }

            if (player.invencivel) {
                player.invencibilidadeTimer -= GetFrameTime();
                if (player.invencibilidadeTimer <= 0) {
                    player.invencivel = false;  // Invencibilidade expirada
                }
            }
            
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].isActive) {
                    if(player.isAttacking && !enemies[i].enemy.invencivel){
                        if(enemies[i].enemy.lives > 0){
                            enemies[i].enemy.state = HURT;
                            enemies[i].enemy.lives--;
                            enemies[i].enemy.invencivel = true;  // Ativa invencibilidade temporária
                            enemies[i].enemy.invencibilidadeTimer = 0.5f;  // Define um tempo de invencibilidade de 1 segundo
                        }
                        else if(enemies[i].enemy.lives == 0){
                            enemies[i].enemy.isAttacking = false;
                            enemies[i].enemy.isDead = true;
                        }
                    }
                    if(enemies[i].enemy.isAttacking && !player.invencivel && !player.isAttacking){
                        if (player.lives > 0) {
                            player.lives--;
                            player.invencivel = true;  // Ativa invencibilidade temporária
                            player.invencibilidadeTimer = 1.0f;  // Define um tempo de invencibilidade de 1 segundo
                        }
                        else if(player.lives == 0){
                            isGameOver = true;
                            player.isDead = true;
                        }
                    }

                    if (enemies[i].enemy.invencivel) {
                        enemies[i].enemy.invencibilidadeTimer -= GetFrameTime();
                        if (enemies[i].enemy.invencibilidadeTimer <= 0) {
                            enemies[i].enemy.invencivel = false;  // Invencibilidade expirada
                        }
                    }

                }
                if(colidiuHand && !player.invencivel){
                    if (player.lives > 0) {
                        player.lives--;
                        player.invencivel = true;  // Ativa invencibilidade temporária
                        player.invencibilidadeTimer = 1.0f;  // Define um tempo de invencibilidade de 1 segundo
                    }
                    else if(player.lives == 0){
                        isGameOver = true;
                        player.isDead = true;
                    }
                } 
            }
            DrawLives(player, camera);
            DrawTimer(camera, elapsedTime);

            DrawTexture(finalfloor_texture, 12600, screenHeight - 230, WHITE);
            if (isGameOver){
                EndMode2D();
                ClearBackground(BLACK);
                const char* text = "Game Over!";
                int textWidth = MeasureText(text, 40);

                DrawText(text, 
                        (GetScreenWidth() - textWidth) / 2,  // Center X
                        GetScreenHeight() / 2 - 20,          // Center Y
                        40, 
                        RED);
                
            }
            if(player.x >= 12410) {
                char *text;
                
                if(potionsCollected == 3) {
                    gameState = FINAL;
                } else {
                    text = "\tVocê não coletou todos os\n\nitens necessários para a cura.";
                }
                int textWidth = MeasureText(text, 40);
                DrawText(text,
                    (12530 - textWidth),  // X
                    screenHeight / 2 - 20, // Y
                    40,
                    RED);
            }
        }
        else if (gameState == FINAL) {
            // Mensagem principal
            const char *message = "Parabéns!\n\n\nVocê chegou no abrigo e coletou todos\n\nos itens para ajudar os cientistas\n\nà produzir a cura";
            int textWidth = MeasureText(message, 40);

            BeginDrawing();
            ClearBackground(RAYWHITE);

            // Desenhar o título e a mensagem final
            DrawTexture(backgroundTitle, 0, 0, WHITE);
            DrawText(message,
                    (GetScreenWidth() - textWidth) + 20, // X
                    GetScreenHeight() / 2 - 10, // Y
                    30,
                    GREEN);

            // Capturar entrada de texto
            int ch = GetCharPressed();
            while (ch > 0) {
                if (ch >= 32 && ch <= 126 && letterCount < maxLength) { // Caracteres válidos
                    nome[letterCount] = (char)ch;
                    letterCount++;
                    nome[letterCount] = '\0'; // Atualiza o terminador
                }
                ch = GetCharPressed(); // Captura o próximo caractere
            }
            
            // Apagar com BACKSPACE
            if (IsKeyPressed(KEY_BACKSPACE) && letterCount > 0) {
                letterCount--;
                nome[letterCount] = '\0';
            }
            if (IsKeyPressed(KEY_ENTER)) {
                add_winner(&head, nome, elapsedTime);
                writeWinners();
                foiAdicionado = 1;
            }

            if(foiAdicionado == 1){
                DrawText("Nome adicionado à lista de vencedores!", (GetScreenWidth() - textWidth) + 20, (GetScreenHeight() / 2) + 230, 20, DARKGREEN);
            }

            // Renderizar entrada de nome
            DrawText("Digite seu nome:", (GetScreenWidth() - textWidth) + 20, (GetScreenHeight() / 2) + 150, 20, LIGHTGRAY);
            DrawText(nome, (GetScreenWidth() - textWidth) + 190, (GetScreenHeight() / 2) + 150, 20, LIGHTGRAY);
        }

        EndDrawing();
    }

    // unload texturas
    UnloadTexture(player.idleTexture);
    UnloadTexture(player.deadTexture);
    UnloadTexture(player.runTexture);
    UnloadTexture(player.jumpTexture);
    UnloadTexture(player.attackTexture);
    UnloadTexture(enemy.idleTexture);
    UnloadTexture(enemy.runTexture);
    UnloadTexture(enemy.attackTexture);
    UnloadTexture(backgroundTitle);
    UnloadTexture(floor_texture);
    UnloadTexture(pit2_texture);
    UnloadTexture(background_texture);
    UnloadTexture(background2_texture);
    UnloadTexture(PotionIcon);
    UnloadTexture(finalfloor_texture);
    UnloadMusicStream(music);
    CloseAudioDevice();
   
    CloseWindow();
    return 0;
}
