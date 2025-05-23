#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <sys/ioctl.h>

#define FIELD_HEIGHT 24         //フィールドの高さ(奇数)
#define FIELD_WIDTH 24          //フィールドの幅(奇数)
#define RAD_ADD 3               //角度の増分
#define MV_ADD 0.1              //移動の増分
#define COLLISION 0             //当たり判定
#define VIEW_R 10.0              //視野の長さ
#define VIEW_A 45               //視野角
#define VIEW_ROUGH 1            //視野の粗さ
#define VIEW_HEIGHT 2           //画面上の壁の高さ
#define START_X 3               //スタート位置x
#define START_Y 3               //スタート位置y
#define GOAL_X 21               //ゴール位置x
#define GOAL_Y 21               //ゴール位置y
#define ITEM_SIZE 0.4
#define ITEM_HEIGHT 4
#define INST_WIDTH 45
#define AA_WIDTH 184
#define BOX_HEIGHT 43
#define BOX_WIDTH 84

#define UP 0
#define LEFT 1
#define DOWN 2
#define RIGHT 3

#define toRad(x) (double)x*(M_PI/180.0)
#define colorReset() printf("\x1b[0m")
#define isGoal(x,y) floor(x) == GOAL_X && floor(y) == GOAL_Y

struct Player{
    double x;
    double y;
    int rad;
};

enum game_menu{
    menu_game,
    menu_map,
    menu_clear
};
enum game_menu menu;
struct Player plr;
struct winsize window_size;

//スプライト
const char *inst[2][17]={{
"           _____",
"         /|     |\\",
"         ||  w  ||",
"         ||_____||",
"  _____  |/_____\\|  _____             _____",
"/|     |\\/|     |\\/|     |\\         /|     |\\",
"||  a  ||||  s  ||||  d  ||         ||  m  ||",
"||_____||||_____||||_____||         ||_____||",
"|/_____\\||/_____\\||/_____\\|         |/_____\\|",
"            移動                   マップ切り替え",
"",
"",
"                             _____回転_____",
"                           /|     |\\/|     |\\",
"                           ||  j  ||||  k  ||",
"                           ||_____||||_____||",
"                           |/_____\\||/_____\\|"
},
"           _____",
"         /|     |\\",
"         ||     ||",
"         ||_____||",
"  _____  |/_____\\|  _____             _____",
"/|     |\\/|     |\\/|     |\\         /|     |\\",
"||     ||||     ||||     ||         || map ||",
"||_____||||_____||||_____||         ||_____||",
"|/_____\\||/_____\\||/_____\\|         |/_____\\|",
"                                   マップ切り替え",
"",
"",
"                             _____    _____",
"                           /|     |\\/|     |\\",
"                           ||     ||||     ||",
"                           ||_____||||_____||",
"                           |/_____\\||/_____\\|"
};
const char *clear_mess[]={
"                                                                                                  d888b,",
"                                                                                                 d888888b",
"                                                                                                d88888888",
"                                                                                               d@@@@8888p",
"                                                                                              / / \"@@@@P",
"                                                                                             |  ,  ,/ /",
"                                                                                             \\_--._ `/",
"                                                                                             /    `\\/",
"                                                                                             |   ,/'           .,.",
"                                                                                        ____/    /           .d888b",
"                                                                         __...----'''```d88/   /``--.__    .@888888[",
"                                                                    _,-``   .dMMM.   .d888/   /8b      `'.' /@@@@@P",
"                                                                 .-`     .dMMMMMMM   8888/   /888   ,@@.` ,  / q@P",
"                                                               .`        MMMMMMMMM   888/   /888\"  @@@/__` /  / P",
"                                                             .`          YMMMMMMMP   Y8/   /8P\"    @,`   ', ,','``-._",
"                                                           .`             YMMMMM      /   /        /      ;,-'       `-_",
"                                                          ; `                        /   /        ,-.  _,/@@*   ;&&L    `\\",
"                                                        ,;                          /   /       ,'  ,';@@@\"  ,&&&&&&&&b  `\\",
"                                                       ,'                          /   /      ,'  ,'         &&&&&&&&&&&   |",
"                                                      ,`                          /   /     ,'  ,'           \"&&&&&&&&&`   |",
"                                                     ;                           /   /    ,'  ,'                `&&&&\"      \\",
"                                                    /                           /   /   ,'  ,'                  ::::::.     |",
"                                                    |                          /   /  ,'  ,'                   !:::::::::   |.",
"                                                   .'                         /   / ,'  ,'                     ::::::::::   `|",
"                                                   |                         /   /,'  ,'                       '!!!!!:::!    |",
"                                                   |                        /   ,'  ,'                           '!!!!:'    ./",
"                                                   |                       /  ,'  ,'                       _.._           .//",
"                                                  .`                      / ,'  ,'                      .d######b.       / /",
"                                                  |                       ,'  ,'                        #########[     ./ /",
"                                                  |                     ,' ,-`                   ..,,   Y########\"   ./  /",
"                                                  |                   ,' ,'/                   d$$$$$$b  `Y#####P  ,/  ,`",
"                                                   \\                ,' ,' /.__                $$$$$$$$$[    `'   .'  .`",
"                                                    \\`.           ,' ,'  /.._ `\\       _.--.  Y$$$$$$$\"      _.-` .-`",
"                                                     `.`.       ,' ,'   /    `-|     ,' ___ \\  Y$$$$P  _..-'` _.-`",
"                                                       `.`.   ,' ,;/   /       /    ( .`   `|    _..-'`   _.-`",
"                                                         ',';' ,' /   /-.._.-'` __ -`]      (-'``   _.--`'",
"                                                          ,' ,'__/   /____..--``  _.`        `..--'`",
"                                                        ,' ,'`-_/   /_____....--'`",
"                                                      ,' ,'    /   /",
"                                                    ,' ,'     /   /",
"                                                  ,' ,'      /   /",
"                                                 ; ,`       /   /",
"                                                 '`         \\__/",
"",
"                __   __  _____  _     _   _______  _____  _     _ __   _ ______    _______    _____  _______        _______ _______ _______ _______   /",
"                  \\_/   |     | |     |   |______ |     | |     | | \\  | |     \\   |_____|   |_____] |_____| |      |______    |       |    |______  / ",
"                   |    |_____| |_____|   |       |_____| |_____| |  \\_| |_____/   |     |   |       |     | |_____ |______    |       |    |______ .  ",
"                      _______ _______ . _______    _____  _______ _____ __   _ _______   _______ _     _ _______   _  _  _  _____   ______        ______ ",
"               |      |______    |    ' |______   |_____] |_____|   |   | \\  |    |         |    |_____| |______   |  |  | |     | |_____/ |      |     \\",
"               |_____ |______    |      ______|   |       |     | __|__ |  \\_|    |         |    |     | |______   |__|__| |_____| |    \\_ |_____ |_____/",
" _  _  _ _____ _______ _     _   _______  _____          _____   ______   _____ __   _   _______ _     _ _______   __   _ _______ _     _ _______    ______  _____   _____  _______   /",
" |  |  |   |      |    |_____|   |       |     | |      |     | |_____/     |   | \\  |      |    |_____| |______   | \\  | |______  \\___/     |      |_____/ |     | |     | |  |  |  / ",
" |__|__| __|__    |    |     |   |______ |_____| |_____ |_____| |    \\_   __|__ |  \\_|      |    |     | |______   |  \\_| |______ _/   \\_    |      |    \\_ |_____| |_____| |  |  | ."};

char *box[4][43] = {
{
"",
"",
"",
"",
"",
"",
"",
"             _.----..       .---.                 .---.       ..----._",
"           ,`    .;;.\\____.`   .`\\_______________/'.   `.____/.;;.    '.",
"         /` .  ,` ,`-----/` . / .`---------------`. \\ . `\\-----`. `.  . `\\",
"       ,`    ,` ,`------/    / /-------------------\\ \\    \\--===-`. `.    `.",
"      .` o ,; ,`       / o  / /   =/                \\ \\  o \\       `. :, o `.",
"     /    .` :-------=/    / /============-----------\\ \\    \\--------: `.    \\",
"    /    / ,/ **     |    / /               .888888b  \\ \\    |        \\. \\    \\",
"   /    /  /        ,|   | ,|             ]MMMMMMMMP  |. |   |,        \\  \\    \\",
"  /    ,/ |         |'  ,| /'             \"Y8888P     `\\ |.   |         | \\,    \\",
"  | O  |`/`---------| O |',|---------=================-|,`| O |-----==---\\`|  O |",
" .|    |,;          |   | |'                           `| |   |          :.|    |.",
" /'   |\"/           |   | |--..__               ``----..| |   |           \\\"|  `\\",
" |    ||       ..   |   | |      ``               .._/. | |   |     @      ||    |",
" ;    ||____________|   |_|_____________________________|_|   |____________||    ;",
",|                                                                               |.",
"|  O        O         O        O         O         O        O         O        O  |",
"|_________________________________________________________________________________|",
"|                                                                                 |",
"| O         O         O        O        _._        O        O         O         O |",
"|   .________________   ________.      d888b      ,________   ________________.   |",
"|   ||    /         |   ||b     |      Y888P      |      ||   |         /    ||   |",
"|   ||   /\\         |   ||Mb.   |       \"8\"       |      ||   |  --..__/     ||   |",
"\"| O || /           | O ||MMb   \\        T        /      || O |     /  `'-  || O |\"",
" |   ||             |   || \"\"   '=================\"      ||   |    /        ||   |",
" |   ||------=======|   ||-------------------------------||   |---+---------||   |",
" |   ||             |   ||                               ||   |_..,,,-      ||   |",
" |   ||   |         | O ||     __..=                     || O |MMMP`        ||   |",
" \"| O ||  |,        |,  `||--``          --..__         ||'  .|MP\"         || O |\"",
"  |   ||   |        `|   ||                    ''---    ||   |'            ||   |",
"  |   ||---+---------|   ||------------------=======----||   |-------------||   |",
"  |   ||   |         |   ||                dMMMMMMM     ||   |             ||   |",
"  |   ||   `|        | O ||               dMMMMMP\"      || O |             ||   |",
"  \"| O ||            |   ||             .dMP\"           ||   |            || O |\"",
"   |   ||____________|   ||_____________________________||   |____________||   |",
"   |          O        O        O        O        O        O        O          |",
"   |___________________________________________________________________________|"
},
{
"",
"",
"",
"",
"             _.----..       .---.                 .---.       ..----._",
"          .,` _.-;;;.\\____.`   .`\\_______________/'.   `.____/.;;:-._ '.",
"        .`` .:`,`_,`-----/` . /.``---------------``.\\ . `\\-----`.._.`:  ``.",
"      .` o ,; ,``      / o  / /   =/                \\ \\  o \\       `. :, o `.",
"     /    .` :-------=/    / /============-----------\\ \\    \\--------: `.    \\",
"    /    / ,/ **     /    / /               .888888b  \\ \\    \\        \\. \\    \\",
"   /   ,/ |         |'  ,| /'             \"Y8888P     `\\ |.   |         | \\,   \\",
"  | O  |`/`---------| O |',|---------=================-|,`| O |-----==---\\`|  O |",
" .|   .`,;          |   | |'                           `| |   |     @    : `.   |.",
" |    ||`      ..   |   | |      ``               .._/. | |   |           `||    |",
" ;    ||____________|   |_|_____________________________|_|   |____________||    ;",
",|                                                                               |.",
"|  O        O         O        O         O         O        O         O        O  |",
"|_________________________________________________________________________________|",
"     /  /  | \"MMMMMP-            /``---.                               |  \\  \\",
"    /  /   |    \"\"              /       \\                              |   \\  \\",
"   /  /    |.-----=====------------------+-----------------========---.|    \\  \\",
"  /  /    /`|                                                         |`\\    \\  \\",
" /  /____/__|_________________________________________________________|__\\____\\  \\",
"/_________________________________________________________________________________\\",
"|                                                                                 |",
"| O         O         O        O        _._        O        O         O         O |",
"|   .________________   ________.      d888b      ,________   ________________.   |",
"|   ||    /         |   ||b     |      Y888P      |      ||   |         /    ||   |",
"|   ||   /\\         |   ||Mb.   |       \"8\"       |      ||   |  --..__/     ||   |",
"\"| O || /           | O ||MMb   \\        T        /      || O |     /  `'-  || O |\"",
" |   ||             |   || \"\"   '=================\"      ||   |    /        ||   |",
" |   ||------=======|   ||-------------------------------||   |---+---------||   |",
" |   ||             |   ||                               ||   |_..,,,-      ||   |",
" |   ||   |         | O ||     __..=                     || O |MMMP`        ||   |",
" \"| O ||  |,        |,  `||--``          --..__         ||'  .|MP\"         || O |\"",
"  |   ||   |        `|   ||                    ''---    ||   |'            ||   |",
"  |   ||---+---------|   ||------------------=======----||   |-------------||   |",
"  |   ||   |         |   ||                dMMMMMMM     ||   |             ||   |",
"  |   ||   `|        | O ||               dMMMMMP\"      || O |             ||   |",
"  \"| O ||            |   ||             .dMP\"           ||   |            || O |\"",
"   |   ||____________|   ||_____________________________||   |____________||   |",
"   |          O        O        O        O        O        O        O          |",
"   |___________________________________________________________________________|"
},
{
"",
"",
"",
"",
"         _______       ___                              ___        _______",
"       .`   .` _\\_____/   /\\___________________________/\\   \\_____/_ `.   `.",
"     .`   .` .`----==.|  .|===========----------------=-|.  |.--====`. `.   `.",
"   .`   .` .`    ..  |`  |`     ``              ``\\__.. `|  `|   @    `. `.   `.",
"  /    /__/__________|   |_______________________________|   |__________\\__\\    \\",
"./                                                                               \\.",
"|_________________________________________________________________________________|",
" `.   _______________________________________________________________________   .`",
"   `. `;:::/---======-----===================================----------\\:::;` .`",
"     `, `:/_____________________________________________________________\\;' ,`",
"       `._________________________________________________________________,`",
"        /  _____________________________________________________________  \\",
"       /  /|                \\                                          |\\  \\",
"      /  / |MMMMM..          \\____                                     | \\  \\",
"     /  /  | \"MMMMMP-            /``---.                               |  \\  \\",
"    /  /   |    \"\"              /       \\                              |   \\  \\",
"   /  /    |.-----=====------------------+-----------------========---.|    \\  \\",
"  /  /    /`|                                                         |`\\    \\  \\",
" /  /____/__|_________________________________________________________|__\\____\\  \\",
"/_________________________________________________________________________________\\",
"|                                                                                 |",
"| O         O         O        O        _._        O        O         O         O |",
"|   .________________   ________.      d888b      ,________   ________________.   |",
"|   ||    /         |   ||b     |      Y888P      |      ||   |         /    ||   |",
"|   ||   /\\         |   ||Mb.   |       \"8\"       |      ||   |  --..__/     ||   |",
"\"| O || /           | O ||MMb   \\        T        /      || O |     /  `'-  || O |\"",
" |   ||             |   || \"\"   '=================\"      ||   |    /        ||   |",
" |   ||------=======|   ||-------------------------------||   |---+---------||   |",
" |   ||             |   ||                               ||   |_..,,,-      ||   |",
" |   ||   |         | O ||     __..=                     || O |MMMP`        ||   |",
" \"| O ||  |,        |,  `||--``          --..__         ||'  .|MP\"         || O |\"",
"  |   ||   |        `|   ||                    ''---    ||   |'            ||   |",
"  |   ||---+---------|   ||------------------=======----||   |-------------||   |",
"  |   ||   |         |   ||                dMMMMMMM     ||   |             ||   |",
"  |   ||   `|        | O ||               dMMMMMP\"      || O |             ||   |",
"  \"| O ||            |   ||             .dMP\"           ||   |            || O |\"",
"   |   ||____________|   ||_____________________________||   |____________||   |",
"   |          O        O        O        O        O        O        O          |",
"   |___________________________________________________________________________|"
},
{
"               ____    ____                             ____    ____",
"          _.-';.--;___/ * /`\\_________________________/'\\ * \\___;--.;'-._",
"        ,'  ,`_______/   /_/___________________________\\_\\   \\_______`.  `.",
"      .` o      o     o        o         o         o        o     o      o `.",
"     /_______________________________________________________________________\\",
"     |   _________________________________________________________________   |",
"     |. |:::::::::|,                                           .|:::::::::| .|",
"     `| |::::::::::|                                           |::::::::::| |`",
"      |. |:::::::::|==========---------------------------=====-|:::::::::| .|",
"      `| |::::::::/                                   dMMMMb    \\::::::::| |`",
"       |. |::::::/                                   dMMMMMMP    \\::::::| .|",
"       `| |:::::/------------------------======================-- \\::::.| |`",
"        |, |:::'MMMMMP                                             `:::|` |",
"        `| |:'_______________________________________________________`:| |`",
"         |_______________________________________________________________|",
"        /  _____________________________________________________________  \\",
"       /  /|                \\                                          |\\  \\",
"      /  / |MMMMM..          \\____                                     | \\  \\",
"     /  /  | \"MMMMMP-            /``---.                               |  \\  \\",
"    /  /   |    \"\"              /       \\                              |   \\  \\",
"   /  /    |.-----=====------------------+-----------------========---.|    \\  \\",
"  /  /    /`|                                                         |`\\    \\  \\",
" /  /____/__|_________________________________________________________|__\\____\\  \\",
"/_________________________________________________________________________________\\",
"|                                                                                 |",
"| O         O         O        O        _._        O        O         O         O |",
"|   .________________   ________.      d888b      ,________   ________________.   |",
"|   ||    /         |   ||b     |      Y888P      |      ||   |         /    ||   |",
"|   ||   /\\         |   ||Mb.   |       \"8\"       |      ||   |  --..__/     ||   |",
"\"| O || /           | O ||MMb   \\        T        /      || O |     /  `'-  || O |\"",
" |   ||             |   || \"\"   '=================\"      ||   |    /        ||   |",
" |   ||------=======|   ||-------------------------------||   |---+---------||   |",
" |   ||             |   ||                               ||   |_..,,,-      ||   |",
" |   ||   |         | O ||     __..=                     || O |MMMP`        ||   |",
" \"| O ||  |,        |,  `||--``          --..__         ||'  .|MP\"         || O |\"",
"  |   ||   |        `|   ||                    ''---    ||   |'            ||   |",
"  |   ||---+---------|   ||------------------=======----||   |-------------||   |",
"  |   ||   |         |   ||                dMMMMMMM     ||   |             ||   |",
"  |   ||   `|        | O ||               dMMMMMP\"      || O |             ||   |",
"  \"| O ||            |   ||             .dMP\"           ||   |            || O |\"",
"   |   ||____________|   ||_____________________________||   |____________||   |",
"   |          O        O        O        O        O        O        O          |",
"   |___________________________________________________________________________|"
}
};

//迷路の壁か道かを判定する配列．ランダム生成しなくていいならここに直接迷路を入力．0が道，1が壁 (コメントアウトしている迷路は24*24なのでdefine定数に注意する)
int field[FIELD_HEIGHT][FIELD_WIDTH] = {
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1},
{1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
{1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1},
{1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,1},
{1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1},
{1,1,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1},
{1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1},
{1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1},
{1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,0,0,0,0,1},
{1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,1,0,0,0,0,1},
{1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,1},
{1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1},
{1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1},
{1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,1,0,0,1},
{1,0,0,1,0,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1},
{1,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,0,1,1,0,0,1},
{1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,1,0,0,0,1},
{1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
{1,0,0,0,0,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,2,0,1},
{1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,1,1,0,0,0,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
char *ar_ls[] = {"→", "↘", "↓", "↙", "←", "↖", "↑", "↗"};//地図上でのプレイヤーの向きの表示(アスキーでやるなら向きの表示いらないかも)
double dir_rad[] = {0, -M_PI/2, M_PI, M_PI/2};
double wall[2*VIEW_A/VIEW_ROUGH];//壁描画用配列 画面に表示する壁の高さを格納
double item[2*VIEW_A/VIEW_ROUGH];

void setBufferedInput(bool);
void printScreen(void);
void printview(void);
void init(void);
void wallLen(void);
bool isWall(double,double);
bool isItem(double,double);
void color(int);
void colorItem(int);
void createMaze(void);
void dig(int, int);
void clearScreenDraw(void);

void setBufferedInput(bool enable){//キー入力関連 触らないで
    static bool enabled = true;
    static struct termios old;
    struct termios new_;
    if (enable && !enabled){
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        enabled = true;
    }
    else if (!enable && enabled){
        tcgetattr(STDIN_FILENO, &new_);
        old = new_;
        new_.c_lflag &= (~ICANON & ~ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new_);
        enabled = false;
    }
}

void init(void){//初期化
    menu = menu_game;
    struct winsize ws;
    //createMaze();
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    window_size.ws_col = ws.ws_col;
    window_size.ws_row = ws.ws_row;
    system("clear");
    plr.x=START_X + 0.5;
    plr.y=START_Y + 0.5;
    plr.rad=90;
}

void printScreen(void){ 
    const int w = sizeof(wall)/sizeof(double);
    printf("\e[0;0H");//カーソル位置を左上に
    plr.rad = (plr.rad+360) % 360;
    int rad = ((plr.rad + 22)%360) / 45;
    wallLen();//wallに壁までの距離を計算して代入
    switch(menu){
        case menu_game:
            printview();
            for(int i=0;i<17;i++){
                printf("%*s%s\n",(window_size.ws_col-INST_WIDTH)/2,"",inst[menu][i]);
            }
            break;
        case menu_map:
            printf("%*s+",(window_size.ws_col-w)/2-1,"");
            for(int i=0;i<w;i++){
                putchar('-');
            }
            printf("+\n");
            for(int i=0;i<7;i++){
                printf("%*s|%*s|\n",(window_size.ws_col-w)/2-1,"",w,"");
            }
            for(int y=0;y<FIELD_HEIGHT;y++){//地図の表示
            //printf("%*s",(window_size.ws_col-2*FIELD_WIDTH)/2,"");//デバッグ用
            printf("%*s|%*s",(window_size.ws_col-w)/2-1,"",(w-2*FIELD_WIDTH)/2,"");
            for(int x=0;x<FIELD_WIDTH;x++){
                //1マスずつ見て壁なら■，道なら空白，プレイヤーがいればそこに道の代わりにプレイヤーを描画．
                if (field[y][x] == 1) {
                    printf("■ ");
                } else if (field[y][x] == 2) {
                    printf("★ ");
                } else if ((int)floor(plr.x) == x && (int)floor(plr.y) == y) {
                    printf("\e[5m%s\e[0m ", ar_ls[rad]);
                } else {
                    printf("  ");
                }
                // printf("%s",field[y][x]==1?"■ ":field[y][x]==2?"★ ":((int)floor(plr.x)==x&&(int)floor(plr.y)==y)?"\e[0m"+ar_ls[rad]:"\e[0m  ");
                // printf("%s",field[y][x]==1?"\e[48;5;15m  ":field[y][x]==2?"\e[0m色":((int)floor(plr.x)==x&&(int)floor(plr.y)==y)?"\e[0m人":"\e[0m  ");
            }
            printf("\e[0m%*s|",(w-2*FIELD_WIDTH)/2,"");
            putchar('\n');
            printf("\e[K");
            }
            for(int i=0;i<8;i++){
                printf("%*s|%*s|\n",(window_size.ws_col-w)/2-1,"",w,"");
            }
            printf("%*s+",(window_size.ws_col-w)/2-1,"");
            for(int i=0;i<w;i++){
                putchar('-');
            }
            printf("+\n");                       
            for(int i=0;i<17;i++){
                printf("%*s%s\n",(window_size.ws_col-INST_WIDTH)/2,"",inst[menu][i]);
            }
            break;
        default:
            break;
    }
    if(menu!=menu_clear)printf("\e[0J");
}

void printview(void){//壁描画
    const int w = sizeof(wall)/sizeof(double);
    printf("%*s+",(window_size.ws_col-w)/2-1,"");
    for(int i=0;i<w;i++){
        putchar('-');
    }
    printf("+\n");
    for(int y=1;y<VIEW_R*VIEW_HEIGHT;y++){//上側
        int last_col = -1;
        printf("%*s|",(window_size.ws_col-w)/2-1,"");
        for(int x=0;x<w;x++){
            const int col = wall[x] / (VIEW_R / 4);
            const int col_item = item[x] / (VIEW_R / 4);
            if(wall[x]==-1.0){//壁がない
                if(item[x]==-1.0){//壁がないかつアイテムもない
                    colorReset();
                    last_col = col;
                    printf(" ");
                    continue;
                }
                else{//壁がないかつアイテムがある
                    if(last_col != col_item){
                        colorItem(x);
                        last_col = col_item;
                    }
                    printf("%s", ITEM_HEIGHT*item[x]>=y?" ":"#");
                }
            }
            else{//壁がある
                if(last_col != col){
                    color(x);
                    last_col = col;
                }
                if(item[x]==-1.0){//壁があってかつアイテムはない
                    printf("%s", VIEW_HEIGHT*wall[x]>=y?" ":":");
                }
                else{//壁もアイテムもある
                    if(ITEM_HEIGHT*item[x]>=y)printf("%s", VIEW_HEIGHT*wall[x]>=y?" ":":");
                    else{
                        if(last_col != col_item){
                        colorItem(x);
                        last_col = col_item;
                        }
                        printf("%s", ITEM_HEIGHT*item[x]>=y?" ":"#");
                    }
                }
            }
        }
        colorReset();
        putchar('|');putchar('\n');
        printf("\e[K");
    }
    for(int y=VIEW_R*VIEW_HEIGHT;y>0;y--){//下側
        int last_col = -1;
        int last_col_item = -1;
        printf("%*s|",(window_size.ws_col-w)/2-1,"");
        for(int x=0;x<w;x++){
            const int col = wall[x] / (VIEW_R / 4);
            const int col_item = item[x] / (VIEW_R / 4);
            if(wall[x]==-1.0){//壁がない
                if(item[x]==-1.0){//壁がないかつアイテムもない
                    colorReset();
                    last_col = col;
                    printf(" ");
                    continue;
                }
                else{//壁がないかつアイテムがある
                    if(last_col != col_item){
                        colorItem(x);
                        last_col = col_item;
                    }
                    printf("%s", ITEM_HEIGHT*item[x]>=y?" ":"#");
                }
            }
            else{//壁がある
                if(last_col != col){
                    color(x);
                    last_col = col;
                }
                if(item[x]==-1.0){//壁があってかつアイテムはない
                    printf("%s", VIEW_HEIGHT*wall[x]>=y?" ":":");
                }
                else{//壁もアイテムもある
                    if(ITEM_HEIGHT*item[x]>=y)printf("%s", VIEW_HEIGHT*wall[x]>=y?" ":":");
                    else{
                        if(last_col != col_item){
                        colorItem(x);
                        last_col = col_item;
                        }
                        printf("%s", ITEM_HEIGHT*item[x]>=y?" ":"#");
                    }
                }
            }
        }
        colorReset();
        putchar('|');putchar('\n');
        printf("\e[K");
    }
    printf("%*s+",(window_size.ws_col-w)/2-1,"");
    for(int i=0;i<w;i++){
        putchar('-');
    }
    printf("+\n");
}

void color(int x){//壁までの距離からprintf文に色をつける．
    double col = wall[x] / (VIEW_R / 4);
    switch((int)floor(col)){
        case 0:colorReset();break;
        case 1:printf("\e[38;5;247m");break;
        case 2:printf("\e[38;5;242m");break;
        case 3:printf("\e[38;5;238m");break;
    }
}

void colorItem(int x){//壁までの距離からprintf文に色をつける．
    double col = item[x] / (VIEW_R / 4);
    switch((int)floor(col)){
        case 0:colorReset();break;
        case 1:printf("\e[38;5;247m");break;
        case 2:printf("\e[38;5;242m");break;
        case 3:printf("\e[38;5;238m");break;
    }
}

void dig(int i, int j){//穴掘り法
    int up, down, left, right;
    up = 0;
    down = 0;
    left = 0;
    right = 0;
    while (up == 0 || down == 0 || left == 0 || right == 0) {
        int d = rand() % 4;
        switch(d) {
            case UP:
                if (j - 2 >= 0 && j - 2 < FIELD_HEIGHT) {
                    if (field[j - 2][i] == 1) {
                        field[j - 2][i] = 0;
                        field[j - 1][i] = 0;
                        dig(i, j - 2);
                    }
                }
                up++;
                break;
            case DOWN:
                if (j + 2 >= 0 && j + 2 < FIELD_HEIGHT) {
                    if (field[j + 2][i] == 1) {
                        field[j + 2][i] = 0;
                        field[j + 1][i] = 0;
                        dig(i, j + 2);
                    }
                }
                down++;
                break;
            case LEFT:
                if (i - 2 >= 0 && i - 2 < FIELD_WIDTH) {
                    if (field[j][i - 2] == 1) {
                        field[j][i - 2] = 0;
                        field[j][i - 1] = 0;
                        dig(i - 2, j);
                    }
                }
                left++;
                break;
            case RIGHT:
                if (i + 2 >= 0 && i + 2 < FIELD_WIDTH) {
                    if (field[j][i + 2] == 1) {
                        field[j][i + 2] = 0;
                        field[j][i + 1] = 0;
                        dig(i + 2, j);
                    }
                }
                right++;
                break;
        }
    }
}

void createMaze(void) {//迷路を生成
    int i, j;
    srand((unsigned)time(NULL));
    for (j = 0; j < FIELD_HEIGHT; j++) {
        for (i = 0; i < FIELD_WIDTH; i++) {
            field[j][i] = 1;//全部壁で埋める
        }
    }
    i = 2 * (rand() % (FIELD_WIDTH / 2)) + 1;//最初に掘る場所を決める
    j = 2 * (rand() % (FIELD_HEIGHT / 2)) + 1;
    field[j][i] = 0;
    dig(i, j);//穴掘り法で再帰
}

void wallLen(void){//wallに壁までの距離を代入
    for(int i=0;i<sizeof(wall)/sizeof(double);i++){wall[i]=0.0;item[i]=-1.0;}
    for(int ang=plr.rad-VIEW_A; ang<plr.rad+VIEW_A; ang+=VIEW_ROUGH){
        double r=0.1;
        bool item_find = 0;
        while(r <= VIEW_R){
            if(isWall(plr.y+r*sin(toRad(ang)),plr.x+r*cos(toRad(ang))))break;
            if(!item_find && isItem(plr.y+r*sin(toRad(ang)),plr.x+r*cos(toRad(ang)))){
                item_find = 1;
                item[(ang-(plr.rad-VIEW_A))/VIEW_ROUGH] = r * cos((ang - plr.rad)*(M_PI/180.0));
            }
            r+=0.1;
        }
        wall[(ang-(plr.rad-VIEW_A))/VIEW_ROUGH] = r>=VIEW_R?-1.0:(r * cos((ang - plr.rad)*(M_PI/180.0)));
    }
}

bool isWall(double y,double x){//壁を判定
    return field[(int)floor(y)][(int)floor(x)] == 1;
}

bool isItem(double y,double x){//アイテムを判定
    return field[(int)floor(y)][(int)floor(x)] == 2;
}

void clearScreenDraw(void){//クリア画面の描画
    system("clear");
    printf("\e[0;0H");
    for(int i=0;i<BOX_HEIGHT;i++){
        printf("%*s%s\e[0J\n",(window_size.ws_col-BOX_WIDTH)/2,"",box[0][i]);
    }
    printf("\e[0;0H");
    usleep(2000000);
    for(int i=0;i<BOX_HEIGHT;i++){
        printf("%*s%s\e[0J\n",(window_size.ws_col-BOX_WIDTH)/2,"",box[1][i]);
    }
    printf("\e[0;0H");
    usleep(200000);
    for(int i=0;i<BOX_HEIGHT;i++){
        printf("%*s%s\e[0J\n",(window_size.ws_col-BOX_WIDTH)/2,"",box[2][i]);
    }
    printf("\e[0;0H");
    usleep(200000);
    for(int i=0;i<BOX_HEIGHT;i++){
        printf("%*s%s\e[0J\n",(window_size.ws_col-BOX_WIDTH)/2,"",box[3][i]);
    }
    printf("\e[0;0H\e[38;5;246m");
    usleep(300000);
    for(int i=0;i<BOX_HEIGHT;i++){
        printf("%*s%s\e[0J\n",(window_size.ws_col-BOX_WIDTH)/2,"",box[3][i]);
    }
     printf("\e[0;0H\e[38;5;242m");
    usleep(300000);
    for(int i=0;i<BOX_HEIGHT;i++){
        printf("%*s%s\e[0J\n",(window_size.ws_col-BOX_WIDTH)/2,"",box[3][i]);
    }
     printf("\e[0;0H\e[38;5;232m");
    usleep(300000);
    for(int i=0;i<BOX_HEIGHT;i++){
        printf("%*s%s\e[0J\n",(window_size.ws_col-BOX_WIDTH)/2,"",box[3][i]);
    }
    printf("\e[0m\e[0;0H");
    usleep(1000000);
    for(int i=0;i<sizeof(clear_mess)/sizeof(clear_mess[0]);i++){
        printf("%*s%s\n",(window_size.ws_col-AA_WIDTH)/2,"",clear_mess[i]);
    }
    usleep(30000000);
    init();
}


int main(void){
    char c;
    int k;
    struct winsize ws;
    init();
    setBufferedInput(false);
    while(1){
        printScreen();
        k = 0;
        c = getchar();
        switch(c){
            case 'd':       //右
                k++;
            case 's':       //後退
                k++;
            case 'a':       //左
                k++;
            case 'w':       //前進
                //進む先が壁かどうかを判定
                if(menu == menu_game && (!isWall(plr.y + (MV_ADD + COLLISION) * sin(toRad((double)plr.rad) + dir_rad[k]), plr.x + (MV_ADD + COLLISION) * cos(toRad((double)plr.rad) + dir_rad[k])))){
                    plr.x = plr.x + MV_ADD * cos(toRad((double)plr.rad) + dir_rad[k]);
                    plr.y = plr.y + MV_ADD * sin(toRad((double)plr.rad) + dir_rad[k]);
                }
                break;
            case 'i':        //↑キー
                break;
            case 'j':        //←キー
                if(menu == menu_game)plr.rad -= RAD_ADD;
                break;
            case 'l':        //↓キー
                break;
            case 'k':        //→キー
                if(menu == menu_game)plr.rad += RAD_ADD;
                break;
            case 'b'://デバッグ用 壁を生成/削除
                printf("\a");
                field[(int)(plr.y+(sin(toRad(plr.rad))))][(int)(plr.x+(cos(toRad(plr.rad))))] = (field[(int)(plr.y+(sin(toRad(plr.rad))))][(int)(plr.x+(cos(toRad(plr.rad))))] + 1) % 2;
                break;
            case 'g'://デバッグ用 ゴールの前にワープ
                printf("\a");
                plr.x = GOAL_X+0.5;
                plr.y = GOAL_Y-2.5;
                plr.rad = 90;
                break;
            case 'r':
                init();
                break;
            case 'm'://メニューの切り替え
                system("clear");
                switch(menu){
                    case menu_game:
                        menu = menu_map;
                        break;
                    case menu_map:
                        menu = menu_game;
                        break;
                }
                break;
            case 'q'://終了
                system("clear");
                system("stty sane");//画面を戻す
                return 0;
        }
        if(isGoal(plr.x,plr.y)){
            clearScreenDraw();
        }
    }
    return 0;
}
