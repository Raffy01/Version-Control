#include "repo.h"

/*
 * 스테이징 링크드 리스트에서 특정 노드를 찾아 연결을 끊고 메모리를 해제한다.
 * staging *target: 삭제할 대상 노드 포인터
 */
void remove_staging_node(staging *target) {
    // 예외 처리: 리스트가 비어있거나 대상 노드가 NULL인 경우 무시
    if (stag_head == NULL || target == NULL) return;
    
    if (stag_head == target) {
        stag_head = target->next;
    }
    if (target->next != NULL)
        target->next->prev = target->prev;
    if (target->prev != NULL)
        target->prev->next = target->next;
    
    free(target->command);
    free(target->path);
    free(target);
}

/*
 * 새로운 스테이징 노드를 생성하여 전역 링크드 리스트(stag_head)의 끝에 추가한다.
 * char *command: 실행된 명령어 (예: "add", "remove")
 * char *path: 대상 파일 또는 디렉토리 경로
 */
void add_staging_node(char *command, char *path) {
    staging *tmp = (staging *)malloc(sizeof(staging));
    tmp->command = strdup(command);
    tmp->path = strdup(path);
    tmp->next = NULL;
    tmp->prev = NULL;

    if (stag_head == NULL) {
        stag_head = tmp;
    } else {
        staging *curr = stag_head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = tmp;
        tmp->prev = curr;
    }
}

/*
 * 명령어 종류에 따라 스테이징 리스트에 노드를 추가할지(add), 제거할지(remove) 제어한다.
 * char *command: 실행된 명령어
 * char *path: 대상 파일 경로
 */
void update_staging_list(char *command, char *path) {
    if (strcmp(command, "add") == 0) {
        add_staging_node(command, path);
    } else { 
        // 명령어 "remove"인 경우 리스트를 순회하며 일치하는 경로 삭제
        staging *tmp = stag_head;
        while (tmp != NULL) {
            if (strcmp(tmp->path, path) == 0) {
                remove_staging_node(tmp);
                break;
            }
            tmp = tmp->next;
        }
    }
}

/*
 * .staging.log 파일을 읽어들여 스테이징 링크드 리스트를 초기화한다.
 */
void load_staging_log(void) {
    char buf[PATH_LENGTH * 2];
    char character;
    int i = 0;
    
    lseek(fd_stag, 0, SEEK_SET);
    while (read(fd_stag, &character, 1) > 0) {
        buf[i++] = character;
        if (character == '\n') {
            buf[i] = '\0';
            char *command = strtok(buf, " ");
            char *path = strtok(NULL, "\"\n");
            if (command && path) {
                update_staging_list(command, path);
            }
            i = 0;
        }
    }
}

/*
 * 커밋 링크드 리스트에서 특정 노드를 찾아 연결을 끊고 메모리를 해제한다.
 * commit *target: 삭제할 대상 노드 포인터
 */
void remove_commit_node(commit *target) {
    // 예외 처리: 리스트가 비어있거나 대상 노드가 NULL인 경우 무시
    if (commit_head == NULL || target == NULL) return;

    if (commit_head == target) {
        commit_head = target->next;
    }
    if (target->next != NULL)
        target->next->prev = target->prev;
    if (target->prev != NULL)
        target->prev->next = target->next;
    
    free(target->dir);
    free(target->command);
    free(target->path);
    free(target);
}

/*
 * 새로운 커밋 노드를 생성하여 전역 링크드 리스트(commit_head)의 끝에 추가한다.
 * char *dir: 커밋된 버전 디렉토리명
 * char *command: 파일 상태 (예: "new file", "modified", "removed")
 * char *path: 커밋된 대상 원본 경로
 */
void add_commit_node(char *dir, char *command, char *path) {
    commit *tmp = (commit *)malloc(sizeof(commit));
    tmp->dir = strdup(dir);
    tmp->command = strdup(command);
    tmp->path = strdup(path);
    tmp->next = NULL;
    tmp->prev = NULL;

    if (commit_head == NULL) {
        commit_head = tmp;
    } else {
        commit *curr = commit_head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = tmp;
        tmp->prev = curr;
    }
}

/*
 * 파일의 변경 상태에 따라 커밋 리스트에 노드를 추가할지 제어한다.
 * char *dir: 커밋명
 * char *command: 파일 상태 식별자
 * char *path: 대상 파일 경로
 */
void update_commit_list(char *dir, char *command, char *path) {
    if (strcmp(command, "new file") == 0) {
        add_commit_node(dir, command, path);
    } else if (strcmp(command, "removed") == 0) {
        // "removed" 상태인 경우, 이전에 추적하던 목록에서 대상 경로를 삭제
        commit *tmp = commit_head;
        while (tmp != NULL) {
            if (strcmp(tmp->path, path) == 0) {
                remove_commit_node(tmp);
                break;
            }
            tmp = tmp->next;
        }
    }
}

/*
 * .commit.log 파일을 읽어들여 커밋 링크드 리스트를 초기화한다.
 */
void load_commit_log(void) {
    char buf[PATH_LENGTH * 2];
    char character;
    int i = 0;

    lseek(fd_commit, 0, SEEK_SET);
    while (read(fd_commit, &character, 1) > 0) {
        buf[i++] = character;
        if (character == '\n') {
            buf[i] = '\0';
            strtok(buf, "\"");
            char *dir = strtok(NULL, "\"");
            strtok(NULL, " ");
            char *command = strtok(NULL, ":");
            strtok(NULL, "\"");
            char *path = strtok(NULL, "\"\n");
            
            if (dir && command && path) {
                update_commit_list(dir, command, path);
            }
            i = 0;
        }
    }
}

/*
 * 프로그램 시작 시 레포지토리 디렉토리(.repo) 및 로그 파일 존재 여부를 확인하고 초기화한다.
 */
void init_repo(void) {
    // 예외 처리: .repo 디렉토리가 없으면 기본 권한(0777)으로 새로 생성
    if (access(".repo", F_OK) != 0) {
        mkdir(".repo", 0777);
    }
    
    // 예외 처리: 로그 파일을 열거나 생성하는 데 실패한 경우 에러 출력 후 프로그램 종료
    if ((fd_commit = open("./.repo/.commit.log", O_RDWR | O_CREAT, 0666)) < 0) {
        perror("open error for .commit.log");
        exit(1);
    }
    if ((fd_stag = open("./.repo/.staging.log", O_RDWR | O_CREAT, 0666)) < 0) {
        perror("open error for .staging.log");
        exit(1);
    }
    
    load_staging_log();
    load_commit_log();
}

/*
 * 문자열로 입력된 내장 명령어를 식별하여 고유의 정수 코드로 변환한다.
 * char *action: 입력된 명령어 문자열
 * return : int, 명령어에 매칭되는 정수. 매칭 실패 시 -1 반환
 */
int parse_command(char *action) {
    if (!action) return -1;
    if (strcmp(action, "add") == 0) return 1;
    if (strcmp(action, "remove") == 0) return 2;
    if (strcmp(action, "status") == 0) return 3;
    if (strcmp(action, "commit") == 0) return 4;
    if (strcmp(action, "revert") == 0) return 5;
    if (strcmp(action, "log") == 0) return 6;
    if (strcmp(action, "help") == 0) return 7;
    if (strcmp(action, "exit") == 0) return 8;
    return -1;
}

/*
 * add/remove 명령 수행 결과를 .staging.log 파일에 기록한다.
 * char *command: 수행된 명령어
 * char *path: 기록할 대상 경로
 */
void append_staging_log(char *command, char *path) {
    lseek(fd_stag, 0, SEEK_END);
    dprintf(fd_stag, "%s \"%s\"\n", command, path);
}

/*
 * 디렉토리 뎁스(깊이)를 비교하기 위해 주어진 문자열에 포함된 '/' 문자의 개수를 세어 반환한다.
 * char *str: 탐색할 문자열
 * return : int, 발견된 '/'의 개수
 */
int get_path_depth(const char *str) {
    // 예외 처리: NULL 포인터
    if (str == NULL) return 0;
    int count = 0;
    for (int i = 0; i < PATH_LENGTH && str[i] != '\0'; i++) {
        if (str[i] == '/') count++;
    }
    return count;
}

/*
 * .staging.log 기록을 분석하여 해당 경로의 현재 스테이징 상태를 판별한다.
 * const char *target_path: 검사할 절대 경로
 * return : int, 0 = untracked(추적 안 됨), 1 = staged(추가됨), 2 = removed(제거됨)
 */
int get_staging_state(const char *target_path) {
    int fd = open("./.repo/.staging.log", O_RDONLY);
    if (fd < 0) return 0; 
    
    char buf[PATH_LENGTH * 2];
    char character;
    int i = 0;
    int state = 0; 
    
    while (read(fd, &character, 1) > 0) {
        buf[i++] = character;
        if (character == '\n') {
            buf[i] = '\0';
            char *cmd = strtok(buf, " ");
            char *p = strtok(NULL, "\"\n");
            
            if (cmd && p) {
                int len_p = strlen(p);
                // 대상 경로가 로그 경로와 일치하거나 하위 경로인 경우
                if (strncmp(target_path, p, len_p) == 0) {
                    if (target_path[len_p] == '\0' || target_path[len_p] == '/') {
                        if (strcmp(cmd, "add") == 0) state = 1;
                        else if (strcmp(cmd, "remove") == 0) state = 2;
                    }
                }
            }
            i = 0;
        }
    }
    close(fd);
    return state;
}

/*
 * 파일의 MD5 해시값을 계산하여 반환한다.
 * const char *fname: 해시를 계산할 파일의 절대 경로
 * return : unsigned char*, 계산된 해시. 실패 시 NULL
 */
unsigned char* calc_md5_hash(const char *fname) {
    int fd;
    MD5_CTX ctx;
    unsigned char *hash = NULL;
    unsigned char buf[1024]; 
    int length;

    if ((fd = open(fname, O_RDONLY)) < 0) return NULL;
    
    hash = (unsigned char *)malloc(MD5_DIGEST_LENGTH);
    if (!hash) {
        close(fd);
        return NULL;
    }

    MD5_Init(&ctx);
    while ((length = read(fd, buf, sizeof(buf))) > 0) {
        MD5_Update(&ctx, buf, length);
    }
    MD5_Final(hash, &ctx);
    close(fd);
    return hash;
}

/*
 * 해당 파일 경로에 대한 가장 마지막(최신) 커밋 노드를 찾아 반환한다.
 * const char *target_path: 찾을 대상 절대 경로
 * return : commit*, 최신 커밋 노드. 한 번도 커밋되지 않았다면 NULL
 */
commit* get_last_commit(const char *target_path) {
    commit *curr = commit_head;
    commit *last_match = NULL;
    
    // 리스트를 끝까지 순회하면, 마지막에 찾은 노드가 가장 최신 기록이 됨
    while (curr != NULL) {
        if (strcmp(curr->path, target_path) == 0) {
            last_match = curr;
        }
        curr = curr->next;
    }
    return last_match;
}

// BFS 순서 출력을 위한 정렬 함수 (뎁스 오름차순, 같은 뎁스면 이름 오름차순)
int compare_status_file(const void *a, const void *b) {
    StatusFile *f1 = (StatusFile *)a;
    StatusFile *f2 = (StatusFile *)b;
    if (f1->depth != f2->depth) return f1->depth - f2->depth;
    return strcmp(f1->rel_path, f2->rel_path);
}

/*
 * 인자로 받은 경로의 파일과 최신 백업본을 비교하여 변경 상태를 반환한다.
 * const char *path: 절대 경로
 * char *status_out: 상태 문자열을 담을 버퍼
 * return : int, 변경사항이 있으면 1, 없으면 0, 에러시 -1
 */
int check_file_status(const char *path, char *status_out) {
    commit *last = get_last_commit(path);
    struct stat st;
    int current_exists = (lstat(path, &st) == 0 && S_ISREG(st.st_mode));

    // 커밋 기록이 없거나 직전 커밋에서 삭제된 상태일 때
    if (last == NULL || strcmp(last->command, "removed") == 0) {
        if (current_exists) {
            strcpy(status_out, "new file");
            return 1;
        }
        return 0; // 존재하지도 않고 커밋도 없으면 아무 변경 없는 것
    } else { // 백업본이 존재하는 상태
        if (!current_exists) {
            strcpy(status_out, "removed");
            return 1;
        } else {
            // 원본과 백업본 비교 진행
            char cwd[PATH_LENGTH]; 
            if (getcwd(cwd, sizeof(cwd)) == NULL){     
                fprintf(stderr, "unable to get cwd\n");
                return -1;
            }
            char backup_path[PATH_LENGTH * 2];
            // 백업 경로 조립 (.repo/버전명/상대경로)
            snprintf(backup_path, sizeof(backup_path), "%s/.repo/%s%s", cwd, last->dir, path + strlen(cwd));
            
            struct stat b_st;
            if (lstat(backup_path, &b_st) < 0) {
                strcpy(status_out, "modified"); return 1;
            }
            // 1차 비교: 파일 크기
            if (st.st_size != b_st.st_size) {
                strcpy(status_out, "modified"); return 1;
            }
            // 2차 비교: MD5 해시값
            unsigned char *hash1 = calc_md5_hash(path);
            unsigned char *hash2 = calc_md5_hash(backup_path);
            int diff = 1;
            if (hash1 && hash2) {
                diff = memcmp(hash1, hash2, MD5_DIGEST_LENGTH);
            }
            free(hash1); free(hash2);

            if (diff != 0) {
                strcpy(status_out, "modified"); return 1;
            }
            return 0; // unchanged
        }
    }
}

/*
 * 파일 탐색 및 상태 분류 로직이다.
 * CWD의 모든 파일과 커밋 기록을 대조하여 상태를 판별하고 정렬된 배열로 반환한다.
 * StatusFile **staged: 커밋 대기 중인 파일 배열 포인터
 * int *staged_cnt: 커밋 대기 중인 파일 개수 포인터
 * StatusFile **untracked: 추적되지 않는 파일 배열 포인터
 * int *untracked_cnt: 추적되지 않는 파일 개수 포인터
 */
void get_status_arrays(StatusFile **staged, int *staged_cnt, StatusFile **untracked, int *untracked_cnt) {
    // 커밋 대기중인 파일 이름 배열
    *staged = malloc(5000 * sizeof(StatusFile));
    // 추적되지 않는 파일 이름 배열
    *untracked = malloc(5000 * sizeof(StatusFile));
    // 각 개수 포인터(리턴될 것임)
    *staged_cnt = 0; *untracked_cnt = 0;

    char cwd[PATH_LENGTH]; 
        if (getcwd(cwd, sizeof(cwd)) == NULL)
            fprintf(stderr, "unable to get cwd");
    // 모든 파일 경로를 담을 배열 포인터
    char **all_paths = malloc(10000 * sizeof(char*)); int path_count = 0;

    // CWD 내의 모든 정규 파일 수집 준비
    char **queue = malloc(10000 * sizeof(char*)); 
    int head = 0, tail = 0;
    queue[tail++] = strdup(cwd);

    // CWD 내의 모든 정규 파일 수집
    while (head < tail) {
        char *current_dir = queue[head++]; struct dirent **namelist;
        int n = scandir(current_dir, &namelist, NULL, alphasort);
        if (n < 0) { free(current_dir); continue; }

        for (int i = 0; i < n; i++) {
            if (strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..") == 0 ||
                strcmp(namelist[i]->d_name, ".repo") == 0 || strcmp(namelist[i]->d_name, "repo") == 0) {
                free(namelist[i]); continue;
            }
            
            char full_path[PATH_LENGTH * 2];
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, namelist[i]->d_name);
            
            struct stat st;
            if (lstat(full_path, &st) == 0) {
                // 디렉토리
                if (S_ISDIR(st.st_mode)) queue[tail++] = strdup(full_path);
                // 파일
                else if (S_ISREG(st.st_mode)) all_paths[path_count++] = strdup(full_path);
            }
            free(namelist[i]);
        }
        free(namelist); free(current_dir);
    }
    free(queue);
    
    // CWD엔 없지만 커밋 기록에 남아있는 파일 (removed 상태)
    commit *curr = commit_head;
    while (curr != NULL) {
        int exists = 0;
        for (int i = 0; i < path_count; i++) {
            if (strcmp(all_paths[i], curr->path) == 0) { 
                exists = 1; break; 
            }
        }
        if (!exists && strcmp(curr->command, "removed") != 0)
            all_paths[path_count++] = strdup(curr->path);
        curr = curr->next;
    }
    
    // 모든 파일을 순회하며 status와 소속 확인
    for (int i = 0; i < path_count; i++) {
        char *target = all_paths[i];
        int staging_state = get_staging_state(target);
        
        if (staging_state == 2) continue; // removed 처리된 경로는 제외

        char status_str[32];
        if (check_file_status(target, status_str)) {
            StatusFile sf; 
            strcpy(sf.abs_path, target); 
            strcpy(sf.status_str, status_str);
            
            if (strcmp(cwd, target) == 0) { 
                strcpy(sf.rel_path, ".");
            }
            else {
                snprintf(sf.rel_path, sizeof(sf.rel_path), ".%s", target + strlen(cwd));
            }
            sf.depth = get_path_depth(sf.rel_path);

            if (staging_state == 1) { //add
                (*staged)[(*staged_cnt)++] = sf;
            }
            else if (staging_state == 0) { //untracked
                (*untracked)[(*untracked_cnt)++] = sf;
            }
        }
    }
    
    // 자원 정리
    for (int i = 0; i < path_count; i++) free(all_paths[i]);
    free(all_paths);
    
    // 정렬
    qsort(*staged, *staged_cnt, sizeof(StatusFile), compare_status_file);
    qsort(*untracked, *untracked_cnt, sizeof(StatusFile), compare_status_file);
}

/*
 * 커밋 수행 결과를 .commit.log 파일에 포맷에 맞춰 기록한다.
 * const char *dir: 커밋(버전) 디렉토리 이름
 * const char *command: 파일 상태 식별자 ("new file", "modified", "removed")
 * const char *path: 커밋된 파일의 원본 절대 경로
 */
void append_commit_log(const char *dir, const char *command, const char *path) {
    char buf[PATH_LENGTH * 2];
    snprintf(buf, sizeof(buf), "commit: \"%s\" - %s: \"%s\"\n", dir, command, path);
    lseek(fd_commit, 0, SEEK_END);
    if (write(fd_commit, buf, strlen(buf)) < 0)
        fprintf(stderr, "write error\n");
}

/*
 * 입력된 경로를 파싱하여 존재하지 않는 상위 디렉토리들을 재귀적으로 모두 생성한다.
 * const char *path: 생성할 최하위 파일 또는 디렉토리의 절대 경로
 */
void make_recursive_dir(const char *path) {
    char tmp[PATH_LENGTH + 1]; 
    char *p = NULL;
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    // 경로 끝의 / 제거
    if (tmp[len - 1] == '/') 
            tmp[len - 1] = 0;
    // 경로를 순회하며 디렉토리 생성
    for (p = tmp + 1; *p; p++) {
        // 디렉토리를 발견
        if (*p == '/') {
            // 잠시 '\0'을 사용해 문자열을 끊음.
            *p = 0;
            // 디렉토리 확인
            if (access(tmp, F_OK) != 0) mkdir(tmp, 0777);
            // 원상복구
            *p = '/';
        }
    }
    if (access(tmp, F_OK) != 0) 
        mkdir(tmp, 0777);
}

/*
 * 원본 파일을 대상 경로로 복사하되, 원본의 접근 권한(Mode)과 수정 시간(Time) 메타데이터를 완벽히 유지한다.
 * const char *src: 원본 파일 절대 경로
 * const char *dest: 백업될 대상 절대 경로
 */
void backup_file(const char *src, const char *dest) {

    char tmp[PATH_LENGTH + 1];
    strcpy(tmp, dest);
    // 하위 디렉토리가 있으면 재귀적으로 생성(시도)
    char *p = strrchr(tmp, '/');
    if (p) { 
        // 최종 파일에 접근하는 것 방지
        *p = '\0'; 
        make_recursive_dir(tmp); 
    }
    // source, destination 파일
    int fd_src = open(src, O_RDONLY);
    int fd_dest = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_src >= 0 && fd_dest >= 0) {
        char buf[4096]; int len;
        while ((len = read(fd_src, buf, sizeof(buf))) > 0) {
            if (write(fd_dest, buf, len) < 0) {
                fprintf(stderr, "write error\n");
            }
        }
    }
    if (fd_src >= 0) 
        close(fd_src);
    if (fd_dest >= 0) 
        close(fd_dest);

    struct stat st;
    if (stat(src, &st) == 0) {
        // mode, utime 그대로 유지
        chmod(dest, st.st_mode);
        struct utimbuf ut;
        ut.actime = st.st_atime; ut.modtime = st.st_mtime;
        utime(dest, &ut);
    }
}

/*
 * 주어진 파일의 텍스트 라인 수를 세고, 필요한 경우 각 라인의 문자열 배열을 동적 할당하여 반환한다.
 * const char *path: 읽어들일 파일 경로
 * char ***lines_out: 라인 문자열 배열을 반환받을 포인터 (NULL이면 내부에서 해제)
 * return : int, 파일의 총 라인 수
 */
int count_lines(const char *path, char ***lines_out) {
    // 라인 단위 파싱을 위해 파일 스트림 구조체 사용
    FILE *f = fopen(path, "r");
    // 예외 처리: 파일이 존재하지 않거나 접근 권한이 없어 열기에 실패한 경우 0 반환
    if (!f) 
        return 0;
    // 라인수의 초기 최대 크기 (1000 라인)
    int capacity = 1000;
    int count = 0;
    
    // 각 줄의 라인을 담을 포인터 배열
    char **lines = malloc(capacity * sizeof(char*));
    char buf[PATH_LENGTH]; 
        
    // /n 을 만날 때 까지 버퍼에 저장 (fgets로 자동으로 구현)
    while (fgets(buf, sizeof(buf), f)) {
        // 동적 스케일링
        if (count >= capacity) {
            capacity *= 2; 
            lines = realloc(lines, capacity * sizeof(char*));
        }
        lines[count++] = strdup(buf);
    }
    fclose(f);

    if (lines_out) 
        // 배열의 주소를 넘겨줌 (이후에 free됨)
        *lines_out = lines;
    else {
        // lines_out이 NULL 인 경우 (호출자가 line count만 필요함), 자원 해제
        for (int i=0; i<count; i++) free(lines[i]);
        free(lines);
    }
    // 라인 수 반환
    return count;
}

/*
 * LCS(Longest Common Subsequence) 알고리즘을 활용하여 두 텍스트 파일 간의 라인 추가 및 삭제 횟수를 정밀하게 계산한다.
 * const char *old_path: 과거 백업 파일 경로
 * const char *new_path: 현재 원본 파일 경로
 * int *insertions: 추가된 라인 수를 반환받을 포인터
 * int *deletions: 삭제된 라인 수를 반환받을 포인터
 */
void calc_diff(const char *old_path, const char *new_path, int *insertions, int *deletions) {
    char **old_lines = NULL, **new_lines = NULL;
    int old_count = 0, new_count = 0;
    // 새로운 파일 또는 삭제된 파일인 경우
    if (old_path && access(old_path, F_OK) == 0) old_count = count_lines(old_path, &old_lines);
    if (new_path && access(new_path, F_OK) == 0) new_count = count_lines(new_path, &new_lines);
    
    // 라인 수 동일함
    if (old_count == 0 && new_count == 0) { 
        *insertions = 0; 
        *deletions = 0; 
        return; 
    }
    // old 파일이 빈 파일인 경우
    if (old_count == 0) {
        *insertions = new_count; *deletions = 0;
        for(int i=0; i<new_count; i++) 
            free(new_lines[i]); 
        free(new_lines); 
        return;
    }
    // new 파일이 빈 파일인 경우
    if (new_count == 0) {
        *insertions = 0; *deletions = old_count;
        for(int i=0; i<old_count; i++) 
            free(old_lines[i]); 
        free(old_lines); 
        return;
    }
    
    // 2차원 배열 dp 대신 파일의 두 라인을 통해 LCS 계산
    int *prev = calloc(new_count + 1, sizeof(int));
    int *curr = calloc(new_count + 1, sizeof(int));
    
    for (int i = 1; i <= old_count; i++) {
        for (int j = 1; j <= new_count; j++) {
            if (strcmp(old_lines[i-1], new_lines[j-1]) == 0) curr[j] = prev[j-1] + 1;
            else curr[j] = (prev[j] > curr[j-1]) ? prev[j] : curr[j-1];
        }
        for (int j = 0; j <= new_count; j++) prev[j] = curr[j];
    }

    int lcs = prev[new_count];
    *insertions = new_count - lcs; *deletions = old_count - lcs;

    free(prev); free(curr);
    for(int i=0; i<old_count; i++) 
        free(old_lines[i]); 
    free(old_lines);
    for(int i=0; i<new_count; i++) 
        free(new_lines[i]); 
    free(new_lines);
}
