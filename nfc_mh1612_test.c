#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#define APP_MAJOR_VER 1
#define APP_MINOR_VER 0
#define APP_REVISER_VER 0

#define MH1612_IOC_MAGIC (0xb5)
#define MH1612_QUERY _IOWR(MH1612_IOC_MAGIC, 0x1, unsigned char)
#define MH1612_READ _IOWR(MH1612_IOC_MAGIC, 0x2, unsigned char)
#define MH1612_WRITE _IOW(MH1612_IOC_MAGIC, 0x3, unsigned char)

typedef enum {
	TYPEA_CARD = 0,
	TYPEB_CARD,
}card_type_t;

void print_hex(const char *prefix_str, const unsigned char *ptr, size_t len)
{
	printf("%s", prefix_str);
	while (len--) {
		printf("0x%02x ", *ptr++);
	}
	printf("\n");
}

void usage(char *program)
{
	printf("usage:\n");
	printf("-a [0]:request iso14443a type card [1]:request iso14443b type card\n");
	printf("-b <block_index> <write_data>:write a block of data to iso14443a type card\n");
	printf("-c <block_index>:read a block of data from iso14443a type card\n");
	printf("-h usage\n");
	printf("example:\n");
	printf("request iso14443a type card: ./nfc_mh1612_test -a 0\n");
	printf("write a block of data to block8 of iso14443a type card: ./nfc_mh1612_test -b 8 \n");
	printf("0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0a 0x0b 0x0c 0x0d 0x0e 0x0f\n");
	printf("read a block of data from block8 of iso14443a type card: ./nfc_mh1612_test -c 8\n");
	printf("request iso14443b type card: ./nfc_mh1612_test -a 1\n");
}

int main(int argc, char *argv[])
{
	int i = 0;
	int opt = 0;
	int nfc_ctrl = 0;
	unsigned char read_card_cmd[10] = {0x22, 0x04, 0x01, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0x99};
	unsigned char write_card_cmd[26] = {0x23, 0x04, 0x01, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00};
	unsigned char data_buf[32] = {0};

	printf("nfc_mh1612_test version:v%d.%d.%d\n", APP_MAJOR_VER, APP_MINOR_VER, APP_REVISER_VER);

	nfc_ctrl = open("/dev/mh1612", O_RDWR);
	if (nfc_ctrl < 0) {
		perror("Failed to open nfc_ctrl");
		exit(-1);
	}

	while ((opt = getopt(argc, argv, "a:b:c:h")) != -1) {
		switch (opt) {
		case 'a':
			data_buf[0] = atoi(optarg);
			if (ioctl(nfc_ctrl, MH1612_QUERY, data_buf) < 0) {
				printf("Failed to query type %c card\n", data_buf[0] ? 'b': 'a');
				close(nfc_ctrl);
				return errno;
			}
			print_hex("Card ID:", &data_buf[1], data_buf[0]);
			break;
		case 'b':
			write_card_cmd[1] = atoi(optarg);
			for (i = 0; i < 16; i++) {
				write_card_cmd[9+i] = strtoul(argv[3+i], NULL, 16);
			}
			if (ioctl(nfc_ctrl, MH1612_WRITE, write_card_cmd) < 0) {
				printf("Failed to write type a card\n");
				close(nfc_ctrl);
				return errno;
			}
			printf("Write block %d successfully:", write_card_cmd[1]);
			print_hex("", &write_card_cmd[9], 16);
			break;
		case 'c':
			read_card_cmd[1] = atoi(optarg);
			memcpy(data_buf, read_card_cmd, sizeof(read_card_cmd));
			if (ioctl(nfc_ctrl, MH1612_READ, data_buf) < 0) {
				printf("Failed to read type a card\n");
				close(nfc_ctrl);
				return errno;
			}
			printf("Read block %d successfully:", read_card_cmd[1]);
			print_hex("", data_buf, 16);
			break;
		case 'h':
			usage(argv[0]);
			exit(-1);
		default:
			usage(argv[0]);
			exit(-1);
		}
	}

	close(nfc_ctrl);
	return 0;
}

