#ifndef REPO_H
#define REPO_H

#define OPENSSL_API_COMPAT 0x10100000L

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <utime.h>
#include <openssl/md5.h>

#define PATH_LENGTH 4096
#define FILE_LENGTH 255

// 스테이징 구역의 파일 상태를 관리하기 위한 링크드 리스트 노드 구조체
typedef struct staging_node {
    char *command;
    char *path;
    struct staging_node *next;
    struct staging_node *prev;
} staging;

// 커밋된 파일의 상태를 관리하기 위한 링크드 리스트 노드 구조체
typedef struct commit_node {
    char *dir;
    char *command;
    char *path;
    struct commit_node *next;
    struct commit_node *prev;
} commit;
// status와 commit 간 정보 전달을 위한 출력 전용 구조체
typedef struct {
    char abs_path[PATH_LENGTH];
    char rel_path[PATH_LENGTH];
    char status_str[32]; //"new file", "modified", "removed"
    int depth;
} StatusFile;

/* 전역 변수 외부 선언 (main.c에서 실제 정의) */
extern int fd_stag;
extern int fd_commit;
extern staging *stag_head;
extern commit *commit_head;

/* help.c */
void print_usage(int command, int is_error);
void print_usage_all(void);

/* utils.c */
void remove_staging_node(staging *target);
void add_staging_node(char *command, char *path);
void update_staging_list(char *command, char *path);
void load_staging_log(void);
void remove_commit_node(commit *target);
void add_commit_node(char *dir, char *command, char *path);
void update_commit_list(char *dir, char *command, char *path);
void load_commit_log(void);
void init_repo(void);
int parse_command(char *action);
void append_staging_log(char *command, char *path);
int get_path_depth(const char *str);
unsigned char* calc_md5_hash(const char *fname);
int get_staging_state(const char *target_path);
commit* get_last_commit(const char *target_path);
int check_file_status(const char *path, char *status_out);
int compare_status_file(const void *a, const void *b);
void get_status_arrays(StatusFile **staged, int *staged_cnt, StatusFile **untracked, int *untracked_cnt);
void append_commit_log(const char *dir, const char *command, const char *path);
void make_recursive_dir(const char *path);
void backup_file(const char *src, const char *dest);
int count_lines(const char *path, char ***lines_out);
void calc_diff(const char *old_path, const char *new_path, int *insertions, int *deletions);

/* 각 명령어 파일의 메인 진입점 */
void cmd_add(int argc, char *argv[]);
void cmd_remove(int argc, char *argv[]);
void cmd_status(int argc, char *argv[]);
void cmd_commit(int argc, char *argv[]);
void cmd_revert(int argc, char *argv[]);
void cmd_log(int argc, char *argv[]);

#endif
