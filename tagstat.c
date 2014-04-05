#include "id3/tag.h"
#include <stdio.h>
#include <string.h>

enum {
	OPT_TITLE = 1,
	OPT_ARTIST = 2,
	OPT_ALBUM = 4,
	OPT_YEAR = 8
};

enum {
	VAL_FILENAME,
	VAL_TITLE,
	VAL_ARTIST,
	VAL_ALBUM,
	VAL_YEAR,
	VAL_MAX
};

void help(const char *p) {
	printf(
		"Usage: %s [OPTION...] FILE\n"
		"tagstat is an ID3 tag editor for mp3 files.\n"
		"If no options are given, tagstat displays the tags of the file\n"
		"\n"
		"Mandatory arguments to long options are mandatory for short options too.\n"
		"\t-t, --title=TITLE\tSet title\n"
		"\t-a, --artist=ARTIST\tSet artist\n"
		"\t-b, --album=ALBUM\tSet album\n"
		"\t-y, --year=YEAR\t\tSet year\n",
		p
	);
	exit(0);
}

void parse_options(int *options, const char **values, int argc, const char **argv) {
	int i, opt;
	const char *val;
	
	*options = 0;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') break;
		opt = 0;
		val = 0;
		if (argv[i][1] == '-') {
			if (strncmp(argv[i], "--title", 7) == 0) {
				opt = VAL_TITLE; *options |= OPT_TITLE;
			} else if (strncmp(argv[i], "--artist", 8) == 0) {
				opt = VAL_ARTIST; *options |= OPT_ARTIST;
			} else if (strncmp(argv[i], "--album", 7) == 0) {
				opt = VAL_ALBUM; *options |= OPT_ALBUM;
			} else if (strncmp(argv[i], "--year", 6) == 0) {
				opt = VAL_YEAR; *options |= OPT_YEAR;
			}
			val = strchr(argv[i], '=');
			if (val == 0) val = argv[++i]; else val++;
		} else {
			switch (argv[i][1]) {
			case 't': opt = VAL_TITLE; *options |= OPT_TITLE; break;
			case 'a': opt = VAL_ARTIST; *options |= OPT_ARTIST; break;
			case 'b': opt = VAL_ALBUM; *options |= OPT_ALBUM; break;
			case 'y': opt = VAL_YEAR; *options |= OPT_YEAR; break;
			}
			val = argv[++i];
		}
		values[opt] = val;
	}
	values[VAL_FILENAME] = argv[i];
}

void set_tag_text(ID3_Tag *tags, ID3_FrameID id, const char *text) {
	ID3_Frame *fp;
	if ((fp = tags->Find(id)) != NULL) tags->RemoveFrame(fp);
	fp = new ID3_Frame(id);
	if (fp == NULL) {
		fputs("Out of memory!\n", stderr);
		exit(1);
	}
	fp->SetID(id);
	fp->GetField(ID3FN_TEXT)->Set(text);
	tags->AttachFrame(fp);
}

const char *get_tag_text(ID3_Tag *tags, ID3_FrameID id) {
	ID3_Frame *fp;
	if ((fp = tags->Find(id)) == NULL) return 0;
	return fp->GetField(ID3FN_TEXT)->GetRawText();
}

bool file_test(const char *path) {
	FILE *f = fopen(path, "r+");
	if (f == NULL) return false;
	fclose(f);
	return true;
}

int main(int argc, const char **argv) {
	
	int options;
	const char *values[VAL_MAX];
	ID3_Tag tags;

	if (argc < 2) help(argv[0]);
	if (strncmp(argv[1], "-h", 2) == 0) help(argv[0]);

	parse_options(&options, values, argc, argv);

	if (!file_test(values[VAL_FILENAME])) {
		fputs("Cannot open file!\n", stderr);
		return 1;
	}

	tags.Link(values[VAL_FILENAME]);
	
	if (options == 0) {
		printf(
			"Title: %s\n"
			"Artist: %s\n"
			"Album: %s\n"
			"Year: %s\n",
			get_tag_text(&tags, ID3FID_TITLE),
			get_tag_text(&tags, ID3FID_LEADARTIST),
			get_tag_text(&tags, ID3FID_ALBUM),
			get_tag_text(&tags, ID3FID_RECORDINGTIME)
		);
		return 0;	
	}
	if (options & OPT_TITLE) set_tag_text(&tags, ID3FID_TITLE, values[VAL_TITLE]);
	if (options & OPT_ARTIST) set_tag_text(&tags, ID3FID_LEADARTIST, values[VAL_ARTIST]);
	if (options & OPT_ALBUM) set_tag_text(&tags, ID3FID_ALBUM, values[VAL_ALBUM]);
	if (options & OPT_YEAR) set_tag_text(&tags, ID3FID_RECORDINGTIME, values[VAL_YEAR]);
	
	tags.Update();
	
	return 0;
}
