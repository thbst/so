/**
 * SO, 2014
 * Lab #12
 *
 * Task 01 - mini.c
 * Implementing a minimal comand line file utility
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <sys/mount.h>
#include "utils.h"
#include <dirent.h>

#define MAX_LINE_SIZE		256

const char *delim = " \t\n";
char *prompt = "so-lab12";

#define TODO2
#define TODO3
#define TODO4
#define TODO5
#define TODO6
#define TODO7


void listDir(char* path){
	DIR* dir;
	struct dirent *ent;
	if((dir=opendir(path)) != NULL){
		while (( ent = readdir(dir)) != NULL){
			if(strcmp(ent->d_name, ".") != 0  && strcmp(ent->d_name, "..") != 0){
				printf("%s\n", ent->d_name);
				listDir(ent->d_name);
			}
		}
		closedir(dir);
	}
}


void listdir(const char *name, int level)
{
	DIR *dir;
	struct dirent *entry;

	if (!(dir = opendir(name)))
		return;
	if (!(entry = readdir(dir)))
		return;

	do {
		if (entry->d_type == DT_DIR) {
			char path[1024];
			int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
			path[len] = 0;
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			printf("%*s[%s]\n", level*2, "", entry->d_name);
			listdir(path, level + 1);
		}
		else
			printf("%*s- %s\n", level*2, "", entry->d_name);
	} 
	while (entry = readdir(dir));
	closedir(dir);
}


int main(void)
{
	char line[MAX_LINE_SIZE];
	char *cmd, *arg1, *arg2, *arg3;
	int ret; /* to be used for function calls return code */

	while (1) {
		printf("<%s>", prompt);
		fflush(stdout);

		memset(line, MAX_LINE_SIZE, 0);

		if (fgets(line, sizeof(line), stdin) == NULL) {
			exit(EXIT_SUCCESS);
		}

		cmd = strtok(line, delim);
#ifdef DEBUG
		printf("Executing command: %s\n", cmd);
#endif
		if (strncmp(cmd, "quit", 4) == 0) {
			break;
		}
#ifdef TODO2
		/* TODO2: implement list <device_node>
		 * Output: <c|b> <major>:<minor>
		 * e.g: list /dev/zero
		 * Output: /dev/zero: <c> 1:5
		 */
		if (strncmp(cmd, "list", 4) == 0) {
			arg1 = strtok(NULL, delim); /* device_node name */
			if (!arg1)
				continue;
			struct stat buf;
			char c;
			stat(arg1, &buf);
			if (S_ISCHR(buf.st_mode)){
				c = 'c';
			}
			else if (S_ISBLK(buf.st_mode)){
				c = 'b';
			}
			else
				c = 'd';
			printf("%s: <%c> %d:%d\n", arg1, /* type */c, /* major */ major(buf.st_rdev), /* minor */minor(buf.st_rdev));
		}
#endif

#ifdef TODO3
		/* TODO3: implement mount source target fs_type
		 * e.g: mount /dev/sda1 /mnt/disk2 ext3 
		 */

		if (strncmp(cmd, "mount", 5) == 0) {
			arg1 = strtok(NULL, delim); /* source */
			arg2 = strtok(NULL, delim); /* target */
			arg3 = strtok(NULL, delim);/* fs_type (e.g: ext2) */
			ret = mount(arg1, arg2, arg3, 0, NULL);

		}
		if (strncmp(cmd, "umount", 6) == 0) {
			arg1 = strtok(NULL, delim); /* target */
			ret = umount(arg1);
		}	
#endif

#ifdef TODO4
		/* TODO4: implement symlink oldpath newpath
		 * e.g: symlink a.txt b.txt
		 */
		if (strncmp(cmd, "symlink", 7) == 0) {
			arg1 = strtok(NULL, delim); /* oldpath */
			arg2 = strtok(NULL, delim); /* newpath */
			symlink(arg1, arg2);
		}
		if (strncmp(cmd, "unlink", 6) == 0) {
			arg1 = strtok(NULL, delim); /* pathname */
			unlink(arg1);
		}
#endif

#ifdef TODO5
		/* TODO5: implement mkdir pathname mode
		 * e.g: mkdir food 0777
		 */
		if (strncmp(cmd, "mkdir", 5) == 0) {
			arg1 = strtok(NULL, delim); /* pathname */
			arg2 = strtok(NULL, delim); /* mode */
			mkdir(arg1 ,(mode_t)0755);

		}
		if (strncmp(cmd, "rmdir", 5) == 0) {
			arg1 = strtok(NULL, delim); /* pathname */
			rmdir(arg1);

		}
#endif

#ifdef TODO6
		/* TODO6: implement ls dirname
		 * e.g: 
		 */
		if (strncmp(cmd, "ls", 2) == 0) {
			arg1 = strtok(NULL, delim); /* recursively print file starting with arg1 */
			DIR *dirp;
			struct dirent *de;
			dirp = opendir(arg1);

//			while(1) {
//				de = readdir(dirp);
//				if (de == NULL)
//					break;
//				printf("%s\n", de->d_name);
//			}
			listdir(arg1,10);
		}
#endif


#ifdef TODO7
		/* TODO7: implement chdir <dir>
		 * e.g: chdir bar
		 */
		if (strncmp(cmd, "chdir", 5) == 0) {
			arg1 = strtok(NULL, delim); /* pathname */
			chdir(arg1);
		}

		/* TODO7: implement pwdir
		 * e.g: pwd
		 */
		if (strncmp(cmd, "pwd", 3) == 0) {
			/* print workding directory */
			char cwd[1024];
			getcwd(cwd, sizeof(cwd));
			printf("%s\n", cwd);

		}
#endif



	}

	return 0;
}
