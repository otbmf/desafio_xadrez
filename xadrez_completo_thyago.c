#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BOARD_SIZE 8
#define MAX_HISTORY 1024

char board[BOARD_SIZE][BOARD_SIZE];
int white_king_moved = 0, black_king_moved = 0;
int white_rook_moved[2] = {0, 0};
int black_rook_moved[2] = {0, 0};
int fifty_move_counter = 0;
char last_double_pawn_move[3] = "";
char position_history[MAX_HISTORY][65];
int history_count = 0;

void save_game(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++)
            fputc(board[i][j], f);
    }
    fprintf(f, "\n%d %d %d %d %d %d %d %s\n", white_king_moved, black_king_moved,
            white_rook_moved[0], white_rook_moved[1], black_rook_moved[0], black_rook_moved[1],
            fifty_move_counter, last_double_pawn_move);
    fclose(f);
}

void load_game(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++)
            board[i][j] = fgetc(f);
    }
    fscanf(f, "\n%d %d %d %d %d %d %d %s\n", &white_king_moved, &black_king_moved,
           &white_rook_moved[0], &white_rook_moved[1], &black_rook_moved[0], &black_rook_moved[1],
           &fifty_move_counter, last_double_pawn_move);
    fclose(f);
}

int is_valid_pos(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

int is_white(char piece) {
    return isupper(piece);
}

int is_black(char piece) {
    return islower(piece);
}

void save_position_to_history() {
    char snapshot[65] = "";
    int k = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            snapshot[k++] = board[i][j];
        }
    }
    snapshot[k] = '\0';
    strcpy(position_history[history_count++ % MAX_HISTORY], snapshot);
}

int count_repetitions() {
    int count = 0;
    char current[65] = "";
    int k = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            current[k++] = board[i][j];
        }
    }
    current[k] = '\0';
    for (int i = 0; i < history_count; i++) {
        if (strcmp(current, position_history[i]) == 0) count++;
    }
    return count;
}

void init_board() {
    char white[] = {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'};
    char black[] = {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'};

    for (int i = 0; i < BOARD_SIZE; i++) {
        board[0][i] = black[i];
        board[1][i] = 'p';
        board[6][i] = 'P';
        board[7][i] = white[i];
    }

    for (int i = 2; i < 6; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            board[i][j] = '.';
}

void print_board() {
    printf("\n  a b c d e f g h\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%d ", 8 - i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("%d\n", 8 - i);
    }
    printf("  a b c d e f g h\n");
}

int parse_position(const char *pos, int *row, int *col) {
    if (strlen(pos) != 2)
        return 0;
    *col = pos[0] - 'a';
    *row = 8 - (pos[1] - '0');
    return is_valid_pos(*row, *col);
}

int is_path_clear(int row1, int col1, int row2, int col2) {
    int dr = (row2 > row1) - (row2 < row1);
    int dc = (col2 > col1) - (col2 < col1);
    row1 += dr;
    col1 += dc;
    while (row1 != row2 || col1 != col2) {
        if (board[row1][col1] != '.') return 0;
        row1 += dr;
        col1 += dc;
    }
    return 1;
}

int is_valid_move(int from_row, int from_col, int to_row, int to_col, int is_white_turn) {
    if (!is_valid_pos(from_row, from_col) || !is_valid_pos(to_row, to_col)) return 0;
    char piece = board[from_row][from_col];
    char target = board[to_row][to_col];
    if (piece == '.') return 0;
    if (is_white_turn && !is_white(piece)) return 0;
    if (!is_white_turn && !is_black(piece)) return 0;
    if ((is_white_turn && is_white(target)) || (!is_white_turn && is_black(target))) return 0;

    int dr = to_row - from_row;
    int dc = to_col - from_col;

    switch (tolower(piece)) {
        case 'p': {
            int dir = is_white_turn ? -1 : 1;
            int start_row = is_white_turn ? 6 : 1;
            // movimento normal
            if (dc == 0 && dr == dir && target == '.') return 1;
            // primeiro movimento duas casas
            if (dc == 0 && dr == 2*dir && from_row == start_row && board[from_row+dir][from_col] == '.' && target == '.') return 1;
            // captura normal
            if (abs(dc) == 1 && dr == dir && ((is_white_turn && is_black(target)) || (!is_white_turn && is_white(target)))) return 1;
            // en passant será tratado separadamente
            return 0;
        }
        case 'r': {
            if (dr == 0 || dc == 0) {
                if (is_path_clear(from_row, from_col, to_row, to_col)) return 1;
            }
            return 0;
        }
        case 'n': {
            if ((abs(dr) == 2 && abs(dc) == 1) || (abs(dr) == 1 && abs(dc) == 2)) return 1;
            return 0;
        }
        case 'b': {
            if (abs(dr) == abs(dc) && is_path_clear(from_row, from_col, to_row, to_col)) return 1;
            return 0;
        }
        case 'q': {
            if ((dr == 0 || dc == 0 || abs(dr) == abs(dc)) && is_path_clear(from_row, from_col, to_row, to_col)) return 1;
            return 0;
        }
        case 'k': {
            if (abs(dr) <= 1 && abs(dc) <= 1) return 1;
            // Roque tratado separadamente
            return 0;
        }
    }
    return 0;
}

int is_king_in_check(int is_white_turn) {
    int king_row = -1, king_col = -1;
    char king = is_white_turn ? 'K' : 'k';
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == king) {
                king_row = i;
                king_col = j;
                break;
            }
        }
    }
    if (king_row == -1) return 1; // rei não encontrado = xeque (game over)
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            char attacker = board[i][j];
            if (attacker == '.') continue;
            if ((is_white_turn && is_black(attacker)) || (!is_white_turn && is_white(attacker))) {
                if (is_valid_move(i, j, king_row, king_col, !is_white_turn)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Função que valida o movimento e testa se deixa o rei em cheque
int is_valid_move_with_check(int from_row, int from_col, int to_row, int to_col, int is_white_turn) {
    if (!is_valid_move(from_row, from_col, to_row, to_col, is_white_turn)) return 0;

    char piece = board[from_row][from_col];
    char target = board[to_row][to_col];

    // tenta mover temporariamente
    board[to_row][to_col] = piece;
    board[from_row][from_col] = '.';

    // trata roque temporariamente (movimenta a torre também)
    int temp_white_king_moved = white_king_moved;
    int temp_black_king_moved = black_king_moved;
    int temp_white_rook_moved[2] = {white_rook_moved[0], white_rook_moved[1]};
    int temp_black_rook_moved[2] = {black_rook_moved[0], black_rook_moved[1]};

    // Se for rei e movimento de 2 colunas, roque:
    if (tolower(piece) == 'k' && abs(to_col - from_col) == 2 && from_row == to_row) {
        if (is_white_turn) {
            if (to_col == 6) { // roque curto
                board[7][5] = 'R';
                board[7][7] = '.';
            } else if (to_col == 2) { // roque longo
                board[7][3] = 'R';
                board[7][0] = '.';
            }
        } else {
            if (to_col == 6) {
                board[0][5] = 'r';
                board[0][7] = '.';
            } else if (to_col == 2) {
                board[0][3] = 'r';
                board[0][0] = '.';
            }
        }
    }

    int in_check = is_king_in_check(is_white_turn);

    // desfaz movimento temporário
    board[from_row][from_col] = piece;
    board[to_row][to_col] = target;

    // desfaz movimento torre no roque
    if (tolower(piece) == 'k' && abs(to_col - from_col) == 2 && from_row == to_row) {
        if (is_white_turn) {
            if (to_col == 6) {
                board[7][5] = '.';
                board[7][7] = 'R';
            } else if (to_col == 2) {
                board[7][3] = '.';
                board[7][0] = 'R';
            }
        } else {
            if (to_col == 6) {
                board[0][5] = '.';
                board[0][7] = 'r';
            } else if (to_col == 2) {
                board[0][3] = '.';
                board[0][0] = 'r';
            }
        }
    }

    return !in_check;
}

int has_legal_moves(int is_white_turn) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            char piece = board[i][j];
            if ((is_white_turn && is_white(piece)) || (!is_white_turn && is_black(piece))) {
                for (int x = 0; x < BOARD_SIZE; x++) {
                    for (int y = 0; y < BOARD_SIZE; y++) {
                        if (is_valid_move_with_check(i, j, x, y, is_white_turn)) {
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

// Função para promover peão (simples para 'Q')
void promote_pawn(int row, int col, int is_white_turn) {
    if (is_white_turn && row == 0 && tolower(board[row][col]) == 'p') {
        board[row][col] = 'Q';
    }
    if (!is_white_turn && row == 7 && tolower(board[row][col]) == 'p') {
        board[row][col] = 'q';
    }
}

// Atualiza flags de roque ao mover rei ou torre
void update_castling_rights(int from_row, int from_col, int to_row, int to_col) {
    char piece = board[to_row][to_col];
    if (piece == 'K') white_king_moved = 1;
    if (piece == 'k') black_king_moved = 1;
    if (piece == 'R') {
        if (from_row == 7 && from_col == 0) white_rook_moved[1] = 1;
        else if (from_row == 7 && from_col == 7) white_rook_moved[0] = 1;
    }
    if (piece == 'r') {
        if (from_row == 0 && from_col == 0) black_rook_moved[1] = 1;
        else if (from_row == 0 && from_col == 7) black_rook_moved[0] = 1;
    }
}

// Valida se o roque é permitido
int can_castle(int is_white_turn, int side) {
    // side 0 = curto, 1 = longo
    if (is_white_turn) {
        if (white_king_moved) return 0;
        if (side == 0 && white_rook_moved[0]) return 0;
        if (side == 1 && white_rook_moved[1]) return 0;
        if (is_king_in_check(1)) return 0; // rei está em xeque agora
        // Verifica se as casas entre rei e torre estão vazias e não em xeque
        if (side == 0) {
            if (board[7][5] != '.' || board[7][6] != '.') return 0;
            // Verifica se as casas que o rei passa estão em xeque
            for (int c = 4; c <= 6; c++) {
                char temp = board[7][c];
                board[7][4] = '.';
                board[7][c] = 'K';
                if (is_king_in_check(1)) {
                    board[7][c] = temp;
                    board[7][4] = 'K';
                    return 0;
                }
                board[7][c] = temp;
                board[7][4] = 'K';
            }
            return 1;
        } else {
            if (board[7][1] != '.' || board[7][2] != '.' || board[7][3] != '.') return 0;
            for (int c = 2; c <= 4; c++) {
                char temp = board[7][c];
                board[7][4] = '.';
                board[7][c] = 'K';
                if (is_king_in_check(1)) {
                    board[7][c] = temp;
                    board[7][4] = 'K';
                    return 0;
                }
                board[7][c] = temp;
                board[7][4] = 'K';
            }
            return 1;
        }
    } else {
        if (black_king_moved) return 0;
        if (side == 0 && black_rook_moved[0]) return 0;
        if (side == 1 && black_rook_moved[1]) return 0;
        if (is_king_in_check(0)) return 0;
        if (side == 0) {
            if (board[0][5] != '.' || board[0][6] != '.') return 0;
            for (int c = 4; c <= 6; c++) {
                char temp = board[0][c];
                board[0][4] = '.';
                board[0][c] = 'k';
                if (is_king_in_check(0)) {
                    board[0][c] = temp;
                    board[0][4] = 'k';
                    return 0;
                }
                board[0][c] = temp;
                board[0][4] = 'k';
            }
            return 1;
        } else {
            if (board[0][1] != '.' || board[0][2] != '.' || board[0][3] != '.') return 0;
            for (int c = 2; c <= 4; c++) {
                char temp = board[0][c];
                board[0][4] = '.';
                board[0][c] = 'k';
                if (is_king_in_check(0)) {
                    board[0][c] = temp;
                    board[0][4] = 'k';
                    return 0;
                }
                board[0][c] = temp;
                board[0][4] = 'k';
            }
            return 1;
        }
    }
    return 0;
}

void make_move(int from_row, int from_col, int to_row, int to_col, int is_white_turn) {
    char piece = board[from_row][from_col];
    int dr = to_row - from_row;
    int dc = to_col - from_col;

    // Roque
    if (tolower(piece) == 'k' && abs(dc) == 2 && dr == 0) {
        if (is_white_turn) {
            if (dc == 2) { // curto
                board[7][6] = 'K';
                board[7][5] = 'R';
                board[7][4] = '.';
                board[7][7] = '.';
                white_king_moved = 1;
                white_rook_moved[0] = 1;
            } else if (dc == -2) { // longo
                board[7][2] = 'K';
                board[7][3] = 'R';
                board[7][4] = '.';
                board[7][0] = '.';
                white_king_moved = 1;
                white_rook_moved[1] = 1;
            }
        } else {
            if (dc == 2) {
                board[0][6] = 'k';
                board[0][5] = 'r';
                board[0][4] = '.';
                board[0][7] = '.';
                black_king_moved = 1;
                black_rook_moved[0] = 1;
            } else if (dc == -2) {
                board[0][2] = 'k';
                board[0][3] = 'r';
                board[0][4] = '.';
                board[0][0] = '.';
                black_king_moved = 1;
                black_rook_moved[1] = 1;
            }
        }
        fifty_move_counter++;
        strcpy(last_double_pawn_move, "");
        return;
    }

    // En passant
    if (tolower(piece) == 'p' && abs(dc) == 1 && dr == (is_white_turn ? -1 : 1) && board[to_row][to_col] == '.' ) {
        // se movimento diagonal e casa destino vazia, pode ser en passant
        char ep_target = is_white_turn ? 'p' : 'P';
        if (to_row + (is_white_turn ? 1 : -1) >= 0 && to_row + (is_white_turn ? 1 : -1) < BOARD_SIZE) {
            if (board[to_row + (is_white_turn ? 1 : -1)][to_col] == ep_target) {
                // captura en passant
                board[to_row + (is_white_turn ? 1 : -1)][to_col] = '.';
            }
        }
    }

    // Atualiza flags de roque
    update_castling_rights(from_row, from_col, to_row, to_col);

    // Atualiza contador de meio-movimentos para regra dos 50 movimentos
    if (tolower(piece) == 'p' || board[to_row][to_col] != '.') {
        fifty_move_counter = 0;
    } else {
        fifty_move_counter++;
    }

    // Atualiza última jogada de peão dupla para en passant
    if (tolower(piece) == 'p' && abs(dr) == 2) {
        last_double_pawn_move[0] = 'a' + from_col;
        last_double_pawn_move[1] = '8' - from_row;
        last_double_pawn_move[2] = '\0';
    } else {
        strcpy(last_double_pawn_move, "");
    }

    board[to_row][to_col] = piece;
    board[from_row][from_col] = '.';

    // Promoção simplificada para rainha
    promote_pawn(to_row, to_col, is_white_turn);
}

int main() {
    init_board();
    int is_white_turn = 1;
    char input[10];
    save_position_to_history();

    while (1) {
        print_board();
        printf("%s move (ex: e2 e4): ", is_white_turn ? "White" : "Black");
        if (!fgets(input, sizeof(input), stdin)) break;

        if (strncmp(input, "save", 4) == 0) {
            save_game("savegame.dat");
            printf("Game saved.\n");
            continue;
        }
        if (strncmp(input, "load", 4) == 0) {
            load_game("savegame.dat");
            printf("Game loaded.\n");
            continue;
        }

        char from[3], to[3];
        if (sscanf(input, "%2s %2s", from, to) != 2) {
            printf("Invalid input format.\n");
            continue;
        }

        int from_row, from_col, to_row, to_col;
        if (!parse_position(from, &from_row, &from_col) || !parse_position(to, &to_row, &to_col)) {
            printf("Invalid position.\n");
            continue;
        }

        if (!is_valid_move_with_check(from_row, from_col, to_row, to_col, is_white_turn)) {
            printf("Invalid move.\n");
            continue;
        }

        make_move(from_row, from_col, to_row, to_col, is_white_turn);

        save_position_to_history();

        if (is_king_in_check(!is_white_turn)) {
            if (!has_legal_moves(!is_white_turn)) {
                print_board();
                printf("Checkmate! %s wins!\n", is_white_turn ? "White" : "Black");
                break;
            } else {
                printf("Check!\n");
            }
        } else {
            if (!has_legal_moves(!is_white_turn)) {
                print_board();
                printf("Stalemate! Draw!\n");
                break;
            }
        }

        if (fifty_move_counter >= 100) {
            printf("Draw by fifty-move rule.\n");
            break;
        }

        if (count_repetitions() >= 3) {
            printf("Draw by threefold repetition.\n");
            break;
        }

        is_white_turn = !is_white_turn;
    }

    return 0;
}
