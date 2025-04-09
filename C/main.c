#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 600
#define HEIGHT 800

// グレースケールに変換
void convert_to_grayscale(unsigned char input[HEIGHT][WIDTH][3], unsigned char grayscale[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            unsigned char r = input[y][x][0];
            unsigned char g = input[y][x][1];
            unsigned char b = input[y][x][2];
            grayscale[y][x] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
        }
    }
}

// 2値化
void binarize_image(unsigned char grayscale[HEIGHT][WIDTH], unsigned char binary[HEIGHT][WIDTH], unsigned char threshold) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            binary[y][x] = (grayscale[y][x] > threshold) ? 255 : 0;
        }
    }
}

// 重心位置を計算
void calculate_centroid(unsigned char binary[HEIGHT][WIDTH], double *x_g, double *y_g) {
    double sum_x = 0, sum_y = 0, sum_pixel = 0;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (binary[y][x] == 255) { // 白いピクセルのみを考慮
                sum_x += x;
                sum_y += y;
                sum_pixel++;
            }
        }
    }

    if (sum_pixel > 0) {
        *x_g = sum_x / sum_pixel;
        *y_g = sum_y / sum_pixel;
    } else {
        *x_g = -1;
        *y_g = -1;
    }
}

// 画像データを読み込む関数
void load_image(const char *folder_path, const char *file_name, unsigned char **image) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s\\%s", folder_path, file_name); // Windows用
    int width, height, channels;

    // デバッグ: ファイルパス確認
    printf("Loading image from: %s\n", file_path);

    unsigned char *data = stbi_load(file_path, &width, &height, &channels, 3);
    if (data == NULL) {
        fprintf(stderr, "Error loading image: %s\n", stbi_failure_reason());
        exit(EXIT_FAILURE);
    }

    // デバッグ: サイズ確認
    printf("Image loaded: %d x %d, Channels: %d\n", width, height, channels);

    if (width != WIDTH || height != HEIGHT) {
        fprintf(stderr, "Error: expected image size %dx%d, but got %dx%d\n", WIDTH, HEIGHT, width, height);
        stbi_image_free(data);
        exit(EXIT_FAILURE);
    }
    *image = data;
}

// グレースケール画像を保存する関数（JPEG形式）
void save_grayscale_image_as_jpeg(const char *folder_path, const char *file_name, unsigned char grayscale[HEIGHT][WIDTH]) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s\\%s", folder_path, file_name); // Windows用
    if (!stbi_write_jpg(file_path, WIDTH, HEIGHT, 1, grayscale, 100)) {
        fprintf(stderr, "Error writing JPEG file: %s\n", file_path);
        exit(EXIT_FAILURE);
    }
}

// 2値画像を保存する関数（JPEG形式）
void save_binary_image_as_jpeg(const char *folder_path, const char *file_name, unsigned char binary[HEIGHT][WIDTH]) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s\\%s", folder_path, file_name); // Windows用
    if (!stbi_write_jpg(file_path, WIDTH, HEIGHT, 1, binary, 100)) {
        fprintf(stderr, "Error writing JPEG file: %s\n", file_path);
        exit(EXIT_FAILURE);
    }
}

//クロージング処理の画像を保存する関数（JPEG形式）
void save_closed_image_as_jpeg(const char *folder_path, const char *file_name, unsigned char closed[HEIGHT][WIDTH]) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s\\%s", folder_path, file_name); // Windows用
    if (!stbi_write_jpg(file_path, WIDTH, HEIGHT, 1, closed, 100)) {
        fprintf(stderr, "Error writing JPEG file: %s\n", file_path);
        exit(EXIT_FAILURE);
    }
}

// メディアンフィルタによるノイズ除去
void apply_median_filter(unsigned char input[HEIGHT][WIDTH], unsigned char output[HEIGHT][WIDTH]) {
    int window[9];
    for (int y = 1; y < HEIGHT - 1; y++) {
        for (int x = 1; x < WIDTH - 1; x++) {
            int k = 0;
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    window[k++] = input[y + j][x + i];
                }
            }
            // ソートして中央値を取得
            for (int i = 0; i < 9 - 1; i++) {
                for (int j = i + 1; j < 9; j++) {
                    if (window[i] > window[j]) {
                        int temp = window[i];
                        window[i] = window[j];
                        window[j] = temp;
                    }
                }
            }
            output[y][x] = window[4]; // 中央値
        }
    }
}

// 文字「あ」を認識する関数
void recognize_character_a(unsigned char grayscale[HEIGHT][WIDTH], unsigned char binary[HEIGHT][WIDTH]) {       
    unsigned char threshold = 12 ; // しきい値
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (grayscale[y][x] < threshold) {
                binary[y][x] = 255; // 白
            } else {
                binary[y][x] = 0; // 黒
            }
        }
    }
}


// 境界を黒にする関数
void set_border_black(unsigned char image[HEIGHT][WIDTH]) {
    for (int x = 0; x < WIDTH; x++) {
        image[0][x] = 0; // 上辺
        image[HEIGHT - 1][x] = 0; // 下辺
    }
    for (int y = 0; y < HEIGHT; y++) {
        image[y][0] = 0; // 左辺
        image[y][WIDTH - 1] = 0; // 右辺
    }
}



// 点線を引く関数（白色、太線）
void draw_dotted_line(unsigned char image[HEIGHT][WIDTH], int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1; 
    int err = (dx > dy ? dx : -dy) / 10, e2;
    int count = 10;

    while (1) {
        if (count % 20 < 10) { // 点線の描画 (10ピクセルおき)
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int nx = x1 + i;
                    int ny = y1 + j;
                    if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
                        image[ny][nx] = 255; // 白
                    }
                }
            }
        }
        if (x1 == x2 && y1 == y2) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 < dy) { err += dx; y1 += sy; }
        count++;
    }
}

// クロージング処理（膨張と収縮）
void apply_closing(unsigned char input[HEIGHT][WIDTH], unsigned char output[HEIGHT][WIDTH], int iterations) {
    unsigned char temp[HEIGHT][WIDTH];

    for (int iter = 0; iter < iterations; iter++) {
        // 膨張処理
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                unsigned char max_val = 0;
                for (int j = -1; j <= 1; j++) {
                    for (int i = -1; i <= 1; i++) {
                        int ny = y + j;
                        int nx = x + i;
                        if (ny >= 0 && ny < HEIGHT && nx >= 0 && nx < WIDTH) {
                            if (input[ny][nx] > max_val) {
                                max_val = input[ny][nx];
                            }
                        }
                    }
                }
                temp[y][x] = max_val;
            }
        }

        // 収縮処理
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                unsigned char min_val = 255;
                for (int j = -1; j <= 1; j++) {
                    for (int i = -1; i <= 1; i++) {
                        int ny = y + j;
                        int nx = x + i;
                        if (ny >= 0 && ny < HEIGHT && nx >= 0 && nx < WIDTH) {
                            if (temp[ny][nx] < min_val) {
                                min_val = temp[ny][nx];
                            }
                        }
                    }
                }
                output[y][x] = min_val;
            }
        }

        // 次のイテレーションのために出力を入力にコピー
        memcpy(input, output, HEIGHT * WIDTH * sizeof(unsigned char));
    }
}

// オープニング処理（収縮と膨張）
void apply_opening(unsigned char input[HEIGHT][WIDTH], unsigned char output[HEIGHT][WIDTH], int iterations) {
    unsigned char temp[HEIGHT][WIDTH];

    for (int iter = 0; iter < iterations; iter++) {
        // 収縮処理
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                unsigned char min_val = 255;
                for (int j = -1; j <= 1; j++) {
                    for (int i = -1; i <= 1; i++) {
                        int ny = y + j;
                        int nx = x + i;
                        if (ny >= 0 && ny < HEIGHT && nx >= 0 && nx < WIDTH) {
                            if (input[ny][nx] < min_val) {
                                min_val = input[ny][nx];
                            }
                        }
                    }
                }
                temp[y][x] = min_val;
            }
        }

        // 膨張処理
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                unsigned char max_val = 0;
                for (int j = -1; j <= 1; j++) {
                    for (int i = -1; i <= 1; i++) {
                        int ny = y + j;
                        int nx = x + i;
                        if (ny >= 0 && ny < HEIGHT && nx >= 0 && nx < WIDTH) {
                            if (temp[ny][nx] > max_val) {
                                max_val = temp[ny][nx];
                            }
                        }
                    }
                }
                output[y][x] = max_val;
            }
        }

        // 次のイテレーションのために出力を入力にコピー
        memcpy(input, output, HEIGHT * WIDTH * sizeof(unsigned char));
    }
}

int main() {
    // 入力画像
    unsigned char *fig1 = NULL;
    unsigned char (*grayscale_image)[WIDTH] = malloc(HEIGHT * WIDTH * sizeof(unsigned char));
    unsigned char (*binary_image)[WIDTH] = malloc(HEIGHT * WIDTH * sizeof(unsigned char));
    unsigned char (*denoised_image)[WIDTH] = malloc(HEIGHT * WIDTH * sizeof(unsigned char));
    unsigned char (*closed_image)[WIDTH] = malloc(HEIGHT * WIDTH * sizeof(unsigned char));
    unsigned char (*opened_image)[WIDTH] = malloc(HEIGHT * WIDTH * sizeof(unsigned char));

    if (!grayscale_image || !binary_image || !denoised_image || !closed_image || !opened_image) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // 画像データの読み込み処理
    const char *folder_path = "C:\\Users\\neore\\Documents\\C"; // ディレクトリパスを指定
    const char *input_file_name = "fig10.jpg"; // 画像ファイル名を指定

    load_image(folder_path, input_file_name, &fig1);

    // グレースケール変換
    convert_to_grayscale((unsigned char (*)[WIDTH][3])fig1, grayscale_image);

    // グレースケール画像の保存処理（JPEG形式）
    const char *grayscale_output_file_name = "grayscale_image.jpg"; // 保存するファイル名を指定
    save_grayscale_image_as_jpeg(folder_path, grayscale_output_file_name, grayscale_image);

    // メディアンフィルタによるノイズ除去
    apply_median_filter(grayscale_image, denoised_image);

    // ノイズ除去後のグレースケール画像の保存処理（JPEG形式）
    const char *denoised_output_file_name = "denoised_image.jpg"; // 保存するファイル名を指定
    save_grayscale_image_as_jpeg(folder_path, denoised_output_file_name, denoised_image);

    //2値化
    recognize_character_a(denoised_image, binary_image);

    // 2値画像の保存処理（JPEG形式）
    const char *binary_output_file_name = "binary_image.jpg"; // 保存するファイル名を指定
    save_binary_image_as_jpeg(folder_path, binary_output_file_name, binary_image);

    // 境界を黒にする
    set_border_black(binary_image);

    // オープニング処理の繰り返し回数
    int opening_iterations = 5;

    // オープニング処理
    apply_opening(binary_image, opened_image, opening_iterations);

    // 重心位置の計算
    double x_g, y_g;
    calculate_centroid(opened_image, &x_g, &y_g);

    // x軸とy軸に点線を引く
    draw_dotted_line(opened_image, 0, (int)round(y_g), WIDTH - 1, (int)round(y_g)); // y軸の点線
    draw_dotted_line(opened_image, (int)round(x_g), 0, (int)round(x_g), HEIGHT - 1); // x軸の点線

    // 結果の出力
    printf("Centroid: (%.2f, %.2f)\n", x_g, y_g);

    // オープニング処理後の画像を保存
    const char *opened_output_file_name = "opened_image.jpg"; // 保存するファイル名を指定
    save_binary_image_as_jpeg(folder_path, opened_output_file_name, opened_image);

    

    // メモリの解放
    stbi_image_free(fig1);
    free(grayscale_image);
    free(binary_image);
    free(denoised_image);
    free(closed_image);
    free(opened_image);

    return 0;
}
