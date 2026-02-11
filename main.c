#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <mqueue.h>

extern const char *__progname;
static const char *yellow = "\033[0;33m";
static const char *white  = "\033[0;37m";
static const char *cyan   = "\033[0;36m";

static void usage()
{
  printf("Usage: %s [options]\n"
         "Options:\n"
         "  -h                Display this information.\n"
         "  -i <queue name>   Display queue's attributes.\n"
         "  -a                Display all queues' attributes.\n"
         "  -r <queue name>   Remove this queue from the system.\n"
         "  -p                Remove all queues from the system.\n",
         __progname);
}

static void disp_q_attr(const char *qname);
static void disp_all_q_attr();
static void rem_q(const char *qname);
static void rem_all_q();

int main(int argc, char *argv[])
{
  int opt = 0;

  if (1 == argc) {
    usage();
    _exit(1);
  }

  while (-1 != (opt = getopt(argc, argv, "hi:ar:p"))) {
    switch (opt) {
    case 'h':
      usage();
      _exit(0);
      break;
    case 'i':
      disp_q_attr(optarg);
      break;
    case 'a':
      disp_all_q_attr();
      break;
    case 'r':
      rem_q(optarg);
      break;
    case 'p':
      rem_all_q();
      break;
    default:
      printf("Invalid option.\n");
      _exit(1);
      break;
    }
  }

  return 0;
}

void disp_q_attr(const char *qname)
{
  int n = 0;
  mqd_t mqd = -1;
  struct mq_attr attr = { 0, 0, 0, 0 };

  mqd = mq_open(qname, O_RDONLY, S_IRWXU, NULL);
  if (-1 == mqd) {
    if (ENOENT == errno)
      printf("%s%s%s does not exist.\n", yellow, qname, white);
    else if (EINVAL == errno)
      printf("Invalid name format. Did you mean %s/%s%s ?.\n", yellow, qname, white);
    else
      perror("Error opening queue");
    return;
  }
  n = mq_getattr(mqd, &attr);
  if (-1 == n)
    perror("Error getting attributes");
  else
    printf("%s%s%s\n"
           "  flags:        %s% 4ld%s\n"
           "  max num msgs: %s% 4ld%s\n"
           "  max msg size:%s% 4ld%s\n"   /* why one less space here !?!? */
           "  curr msgs:    %s% 4ld%s\n",
           yellow,  qname,            white,
           cyan,    attr.mq_flags,    white,
           cyan,    attr.mq_maxmsg,   white,
           cyan,    attr.mq_msgsize,  white,
           cyan,    attr.mq_curmsgs,  white);
  mq_close(mqd);
  mqd = -1;
}

static int for_all_q(void (*func)(const char *))
{
  DIR *dir = NULL;
  struct dirent *d = NULL;
  char path[PATH_MAX] = { '\0' };
  int n = 0;

  dir = opendir("/dev/mqueue");
  if (!dir) {
    if (ENOENT != errno)
      perror("Failed to open /dev/mqueue directory");
    return 0;
  }

  path[0] = '/';
  while (NULL != (d = readdir(dir))) {
    if (d->d_type == DT_REG) {
      strncpy(&path[1], d->d_name, (sizeof(path) - 2));
      path[sizeof(path) - 1] = '\0';
      func(path);
      n++;
    }
  }
  closedir(dir);
  dir = NULL;

  return n;
}

void disp_all_q_attr()
{
  int n = for_all_q(disp_q_attr);
  if (!n)
    printf("No queues found.\n");
}

void rem_q(const char *qname)
{
  int n = mq_unlink(qname);
  if (-1 == n) {
    if (ENOENT == errno)
      printf("%s%s%s does not exist.\n", yellow, qname, white);
    else
      perror("Error unlinking queue");
  }
  else
    printf("Removed %s.\n", qname);
}

void rem_all_q()
{
  int n = for_all_q(rem_q);
  if (!n)
    printf("No queues found.\n");
}
