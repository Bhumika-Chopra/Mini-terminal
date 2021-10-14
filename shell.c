#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<signal.h>
#include<errno.h>

#define buffersize 128
static char dir[128];
static char str[128];
static char ht[5][128];
static int last = 0;

void set_cwd(){
	char s[128];
	char* p = getcwd(s, 128);
	strcpy(dir,p);
}

void sighandler(int sig) {
	printf("\n");
	exit(0);
}

int cd(char** args, char* line){
	if(args[1] == NULL) 
		return -1;
	else {
		int i,count = 0;
		char* token = strtok(line, " ");
		while(token!=NULL) {
			token = strtok(NULL, " ");
			count++;
		}
		free(token);
		char to[128];
		// strcpy(to, "");
		strcpy(to, args[1]);
		for(i=2; i<count; i++) {
			strcat(to, " ");
			strcat(to, args[i]);
		}
		int j;
		char dest[128];
		strcpy(dest,"");
		for(j=0; j<strlen(to); j++) {
			if(to[j] == '"' || to[j] == '\'') {
				continue;
			}
			strncat(dest, &to[j], 1);
		}

		if(strcmp(args[1], "~") == 0) {
			chdir(dir);
			strcpy(str, "~");
		}
		else if(chdir(dest) == -1) {
			printf("%s\n", strerror(errno));
			return -1;
		}
		char s[128];
		char temp[128];
		char p[128];
		char* pwd = getcwd(s, 128);
		strcpy(p,pwd);

		if (strcmp(p,dir) == 0) {
			strcpy(str, "~");
		}

		else if(strcmp(p,dir) < 0) {
			strcpy(str, p);
		}
		else {
			int m;
			int flag = 1;
			for(m=0; m<strlen(dir); m++) {
				if(dir[m] != p[m]) {
					flag = 0;
					break;
				}
			}
			if(flag) {
				strcpy(str, "~");
				strcpy(temp,str);
				for(m=strlen(dir); m<strlen(p); m++) {
					strncat(temp, &p[m], 1);
				}
				strcpy(str, temp);
			}
			else {
				strcpy(str, p);
			}
			
		}
	}
	return 1;
}

char* read_line(void) {
	int buffer = buffersize;
	int pos = 0;
	char* buf = malloc(buffer*sizeof(char));
	int ch;

	if(!buf) {
		fprintf(stderr, "Memory Allocation by Malloc Error\n");
		exit(1);
	}

	while(1) {
		ch = getchar();
		if(ch==EOF || ch=='\n') {
			buf[pos] = '\0';
			return buf;
		}
		else 
			buf[pos] = ch;
		pos++;

		if(pos >= buffer) {
			return NULL;
		}
	}
}

char** split_line(char* line) {
	int count = 0;
	char* temp = malloc(strlen(line) + 1); 
	strcpy(temp, line);
	char* token = strtok(temp, " ");
	int pos = 0;
	while(token != NULL) {
		count++;
		token = strtok(NULL, " ");
	}
	free(temp);

	char** tokens = malloc((count+1)*sizeof(char*));
	token = strtok(line, " ");
	while(token != NULL) {
		tokens[pos] = token;
		token = strtok(NULL, " ");
		pos++;
	}
	tokens[pos] = NULL;
	return tokens;
}

int default_list(char** args) {
	pid_t rc;
	rc = fork();

	if(rc<0) {
		fprintf(stderr, "Unable to create fork");
	}
	else if(rc == 0) {
		if(execvp(args[0], args) == -1) {
			return -1;
		}
	}
	else {
		pid_t wc = waitpid(rc, NULL, 0);
	}
	return 1;
}

int history(){
	int i = last-5;
	int k=1;
	if(i<0) {
		for(i=0;i<last;i++) {
			printf("%d %s\n", k++, ht[i]);
		}
	}
	else {
		for(i;i<last;i++) {
			printf("%d %s\n", k++, ht[i%5]);
		}
	}
	return 1;
}

int execute(char** args, char* line) {
	if(strcmp(args[0], "cd") == 0) {
		return cd(args, line);
	}
	else if (strcmp(args[0], "history") == 0){
		return history();
	}

	return default_list(args);
}

void new_cmd(char* line){
	char t[128];
	strcpy(t, line);
	strcpy(ht[last%5], t);
	last++;
}

void start_shell(void)
{
  char* line;
  char** args;
  int status = 1;
  set_cwd();
  char s[128];
  char temp[128];
  char temp2[128];
  strcpy(str, "~");
  while(status) {
	strcpy(s, "MTL458:");
	strcat(s,str);
	strcat(s, "$ ");
  	printf("%s", s);
    line = read_line();
    if(line == NULL) {
    	printf("Error: Buffer length exceeded\n");
    	printf("Reenter Command\n");
    	free(line);
    	continue;
    }
    strcpy(temp,line);
    strcpy(temp2,temp);
    new_cmd(temp2);
    args = split_line(line);	
    if(args[0] == NULL) {
    	free(line);
    	free(args);
    	continue;
    }
    status = execute(args, temp);
    if(status==-1) {
		fprintf(stderr, "Error: Incorrect command\n");
		// free(line);
  //   	free(args);
		// continue;
    }
    free(line);
    free(args);
  }

}

int main(int argc, char** argv) {
	signal(SIGINT, sighandler);
	start_shell();
	return 0;
}
