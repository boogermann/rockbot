#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include "file_io.h"
#include "convert.h"
#include "../file/convert.h"
#include "aux_tools/stringutils.h"

#ifdef DREAMCAST
#include <kos.h>
#elif PLAYSTATION2
#include <fileXio_rpc.h>
typedef struct {
    char displayname[64];
    int  dircheck;
    char filename[256];
} entries;
#endif

#ifdef ANDROID
#include <android/log.h>
#include "ports/android/cloud_save.h"
#endif

extern std::string FILEPATH;
extern std::string SAVEPATH;
extern std::string GAMEPATH;
extern std::string GAMENAME;
extern bool GAME_FLAGS[FLAG_COUNT];

// versioned file for config, so we can force resetting it
#define CONFIG_FILENAME "/config_v201.sav"

extern CURRENT_FILE_FORMAT::st_game_config game_config;

// ************************************************************************************************************* //


#ifdef WII
void short_to_little_endian(short &s)
{
     s = (s>>8) | (s<<8);
}

void u_short_to_little_endian(unsigned short &s)
{
    short temp_short = s;
    short_to_little_endian(temp_short);
    s = temp_short;
}

void int_to_little_endian(int &i)
{
    i = (i<<24)|((i<<8)&0xFF0000)|((i>>8)&0xFF00)|(i>>24);
}

void u_int_to_little_endian(unsigned int &s)
{
    int temp_int = s;
    int_to_little_endian(temp_int);
    s = temp_int;
}

void sint16_to_little_endian(int16_t &i)
{
     i = (i << 8) | ((i >> 8) & 0xFF);
}
/*
to convert from big endian to little endian a signed short this function may be used
(from: convert big endian to little endian in C [without using provided func] )
int16_t swap_int16( int16_t val )
{
return (val << 8) | ((val >> 8) & 0xFF);
}
So going back to your code
uint8_t buffer[4];
[inputStream read:buffer maxLength:4];
You may try:
int16_t *value1;
int16_t *value2;
// set the pointers to the correct buffer location
value1 = (int16_t *)(&buffer[0]);
value2 = (int16_t *)(&buffer[2]);
// perform the conversion
*value1 = swap_int16( *value1 );
*value2 = swap_int16( *value2 );
// finally you may want to check the result
printf("Value1: %d\n", *value1 );
printf("Value2: %d\n", *value2 );
Note that this approach will swap the original bytes in buffer



>> MAP::LOAD_NPCS.map[0].id[0] converted-x[193] <<
>> MAP::LOAD_NPCS.map[0].id[1] converted-x[34] <<
>> MAP::LOAD_NPCS.map[0].id[3] converted-x[2] <<
>> MAP::LOAD_NPCS.map[2].id[0] converted-x[17] <<


2 -> 2
6 -> 17
35 -> 34
194 -> 193

*/
#endif

namespace format_v4 {

    file_io::file_io()
    {
        sufix = "_v301";
    }



    void file_io::write_game(format_v4::file_game& data_in) const {
        std::ofstream fp;
        std::string filename = "";


// -------------------------------------- GAME -------------------------------------- //
        filename = std::string(FILEPATH) + "/game_properties" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }



        fp.write(reinterpret_cast<char *>(&data_in.version), sizeof(float));
        fp.write(reinterpret_cast<char *>(&data_in.name), sizeof(char) * FS_CHAR_NAME_SIZE);
        fp.write(reinterpret_cast<char *>(&data_in.semi_charged_projectile_id), sizeof(Sint8));
        fp.write(reinterpret_cast<char *>(&data_in.player_items), sizeof(Sint8) * FS_PLATER_ITEMS_N);
        fp.write(reinterpret_cast<char *>(&data_in.stage_face_filename), sizeof(char) * MAX_STAGES * FS_FACE_FILENAME_MAX);
        fp.write(reinterpret_cast<char *>(&data_in.stages_face_name), sizeof(char) * MAX_STAGES * FS_CHAR8_NAME_SIZE);

        fp.write(reinterpret_cast<char *>(&data_in.boss_music_filename), sizeof(char) * FS_CHAR_NAME_SIZE);
        fp.write(reinterpret_cast<char *>(&data_in.final_boss_music_filename), sizeof(char) * FS_CHAR_NAME_SIZE);
        fp.write(reinterpret_cast<char *>(&data_in.got_weapon_music_filename), sizeof(char) * FS_CHAR_NAME_SIZE);
        fp.write(reinterpret_cast<char *>(&data_in.game_over_music_filename), sizeof(char) * FS_CHAR_NAME_SIZE);
        fp.write(reinterpret_cast<char *>(&data_in.stage_select_music_filename), sizeof(char) * FS_CHAR_NAME_SIZE);
        fp.write(reinterpret_cast<char *>(&data_in.game_start_screen_music_filename), sizeof(char) * FS_CHAR_NAME_SIZE);

        fp.write(reinterpret_cast<char *>(&data_in.use_second_castle), sizeof(bool));
        fp.write(reinterpret_cast<char *>(&data_in.game_style), sizeof(Uint8));
        fp.write(reinterpret_cast<char *>(&data_in.final_boss_id), sizeof(Uint8));

        fp.close();



// -------------------------------------- WEAPONS -------------------------------------- //
        // file_weapon weapons[FS_MAX_WEAPONS]
        filename = std::string(FILEPATH) + "/game_weapons" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.weapons), sizeof(file_weapon) * FS_MAX_WEAPONS);
        fp.close();


// -------------------------------------- TROPHIES -------------------------------------- //
        // st_file_trophy trophies[TROPHIES_MAX]
        filename = std::string(FILEPATH) + "/game_trophies" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.trophies), sizeof(st_file_trophy) * TROPHIES_MAX);
        fp.close();


// -------------------------------------- ARMOR PIECES -------------------------------------- //
        // st_armor_piece armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX]
        filename = std::string(FILEPATH) + "/game_armorPieces" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.armor_pieces), sizeof(st_armor_piece) * FS_PLAYER_ARMOR_PIECES_MAX);
        fp.close();


// -------------------------------------- WEAPON MENU COLORS -------------------------------------- //
        // st_color weapon_menu_colors[MAX_WEAPON_N];
        filename = std::string(FILEPATH) + "/game_weaponMenuColors" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.weapon_menu_colors), sizeof(st_color) * MAX_WEAPON_N);
        fp.close();

    }




    void file_io::write_all_stages(format_v4::file_stages &stages_data_in) const
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "/stages" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_all_stages - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        } else {
            std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
        }
        fp.write(reinterpret_cast<char *>(&stages_data_in), sizeof(struct format_v4::file_stages));
        fp.close();
    }



    void file_io::read_game(format_v4::file_game& data_out) const {
        FILE *fp;
        std::string filename = "";


        bool USE_NEW_READ = true;
        if (USE_NEW_READ == false) {
            filename = std::string(FILEPATH) + "/game" + sufix + ".dat";
            fp = fopen(filename.c_str(), "rb");
            if (!fp) {
                std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
                fflush(stdout);
                return;
            }
            int read_result = fread(&data_out, sizeof(struct format_v4::file_game), 1, fp);
            if (read_result  == -1) {
                std::cout << ">>file_io::read_game - Error reading struct data from game file '" << filename << "'." << std::endl;
                fflush(stdout);
                exit(-1);
            }
            fclose(fp);
            return;
        }
// -------------------------------------- GAME -------------------------------------- //
        // float version;
        // char name[FS_CHAR_NAME_SIZE];
        // Sint8 semi_charged_projectile_id;
        // Sint8 player_items[FS_PLATER_ITEMS_N];
        // char stage_face_filename[MAX_STAGES][FS_FACE_FILENAME_MAX]
        filename = std::string(FILEPATH) + "/game_properties" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }


        if (unsigned int res = fread(&data_out.version, sizeof(float), 1, fp) != 1) {
            std::cout << ">>file_io::read_game - res: " << res << ", sizeof(float): " << sizeof(float) << ", Error reading struct data [version2] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        if (unsigned int res = fread(&data_out.name, sizeof(char), FS_CHAR_NAME_SIZE, fp) != FS_CHAR_NAME_SIZE) {
            std::cout << ">>file_io::read_game - res: " << res << ", sizeof(char): " << sizeof(char) << ", Error reading struct data from [name] game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        if (fread(&data_out.semi_charged_projectile_id, sizeof(Sint8), 1, fp) != 1) {
            std::cout << ">>file_io::read_game - Error reading struct data [semi_charged_projectile_id] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        if (fread(&data_out.player_items, sizeof(Sint8), FS_PLATER_ITEMS_N, fp) != FS_PLATER_ITEMS_N) {
            std::cout << ">>file_io::read_game - Error reading struct data [player_items] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        if (fread(&data_out.stage_face_filename, sizeof(char), (MAX_STAGES*FS_FACE_FILENAME_MAX), fp) != MAX_STAGES*FS_FACE_FILENAME_MAX) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [stage_face_filename] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        if (fread(&data_out.stages_face_name, sizeof(char), (MAX_STAGES*FS_CHAR8_NAME_SIZE), fp) != MAX_STAGES*FS_CHAR8_NAME_SIZE) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [stages_face_name] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }

        if (fread(&data_out.boss_music_filename, sizeof(char), (FS_CHAR_NAME_SIZE), fp) != FS_CHAR_NAME_SIZE) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [boss_music_filename] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        if (fread(&data_out.final_boss_music_filename, sizeof(char), (FS_CHAR_NAME_SIZE), fp) != FS_CHAR_NAME_SIZE) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [final_boss_music_filename] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        if (fread(&data_out.got_weapon_music_filename, sizeof(char), (FS_CHAR_NAME_SIZE), fp) != FS_CHAR_NAME_SIZE) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [got_weapon_music_filename] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        if (fread(&data_out.game_over_music_filename, sizeof(char), (FS_CHAR_NAME_SIZE), fp) != FS_CHAR_NAME_SIZE) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [game_over_music_filename] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        if (fread(&data_out.stage_select_music_filename, sizeof(char), (FS_CHAR_NAME_SIZE), fp) != FS_CHAR_NAME_SIZE) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [stage_select_music_filename] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }

        if (fread(&data_out.game_start_screen_music_filename, sizeof(char), (FS_CHAR_NAME_SIZE), fp) != FS_CHAR_NAME_SIZE) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [stage_select_music_filename] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }

        if (fread(&data_out.use_second_castle, sizeof(bool), 1, fp) != 1) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [use_second_castle] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        if (fread(&data_out.game_style, sizeof(Uint8), 1, fp) != 1) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [game_style] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        if (fread(&data_out.final_boss_id, sizeof(Uint8), 1, fp) != 1) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [final_boss_id] from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }

        fclose(fp);


// -------------------------------------- WEAPONS -------------------------------------- //
        // file_weapon weapons[FS_MAX_WEAPONS]
        filename = std::string(FILEPATH) + "/game_weapons" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.weapons, sizeof(file_weapon), FS_MAX_WEAPONS, fp) != FS_MAX_WEAPONS) {
            std::cout << ">>file_io::read_game[weapons] - Error reading data from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- TROPHIES -------------------------------------- //
        // st_file_trophy trophies[TROPHIES_MAX]
        filename = std::string(FILEPATH) + "/game_trophies" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.trophies, sizeof(st_file_trophy), TROPHIES_MAX, fp) != TROPHIES_MAX) {
            std::cout << ">>file_io::read_game[trophies] - Error reading data from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- ARMOR PIECES -------------------------------------- //
        // st_armor_piece armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX]
        filename = std::string(FILEPATH) + "/game_armorPieces" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.armor_pieces, sizeof(st_armor_piece), FS_PLAYER_ARMOR_PIECES_MAX, fp) != FS_PLAYER_ARMOR_PIECES_MAX) {
            std::cout << ">>file_io::read_game[armor_pieces] - Error reading data from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        fclose(fp);



// -------------------------------------- WEAPON MENU COLORS -------------------------------------- //
        // st_color weapon_menu_colors[MAX_WEAPON_N];
        filename = std::string(FILEPATH) + "/game_weaponMenuColors" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);

        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.weapon_menu_colors, sizeof(st_color), MAX_WEAPON_N, fp) != MAX_WEAPON_N) {
            std::cout << ">>file_io::read_game[armor_pieces] - Error reading data from game file '" << filename << "'." << std::endl;
            fclose(fp);
            exit(-1);
        }
        fclose(fp);
    }


    void file_io::read_all_stages(format_v4::file_stages& stages_data_out)
    {
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + "/stages" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        fp.read(reinterpret_cast<char *>(&stages_data_out), sizeof(struct format_v4::file_stages));
        fp.close();
    }


    void file_io::read_stage(format_v4::file_stage &stages_data_out, short stage_n)
    {
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "/stages" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            fflush(stdout);
            return;
        }
        fseek(fp, sizeof(format_v4::file_stage) * stage_n, SEEK_SET);
        int read_result = fread(&stages_data_out, sizeof(struct format_v4::file_stage), 1, fp);
        if (read_result == -1) {
            printf(">>file_io::read_game - Error reading struct data from stage file.\n");
            fflush(stdout);
            exit(-1);
        }
        fclose(fp);
    }

    void file_io::read_all_maps(file_map (&data_out)[FS_MAX_STAGES][FS_STAGE_MAX_MAPS])
    {
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + std::string("/maps.dat");
        filename = StringUtils::clean_filename(filename);
        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_all_maps - could not load file '" << filename << "'" << std::endl;
            return;
        }
        for (int i=0; i<FS_MAX_STAGES; i++) {
            for (int j=0; j<FS_STAGE_MAX_MAPS; j++) {
                fp.read(reinterpret_cast<char *>(&data_out[i][j]), sizeof(file_map));
            }
        }

        fp.close();
    }

    void file_io::write_all_maps(file_map (&data_in)[FS_MAX_STAGES][FS_STAGE_MAX_MAPS])
    {
        std::string filename = std::string(FILEPATH) + "/maps.dat";
        std::ofstream fp;
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_all_maps - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        } else {
            std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
        }
        for (int i=0; i<FS_MAX_STAGES; i++) {
            for (int j=0; j<FS_STAGE_MAX_MAPS; j++) {
                fp.write(reinterpret_cast<char *>(&data_in[i][j]), sizeof(file_map));
            }
        }
        fp.close();
    }

    void file_io::read_stage_maps(int stage_id, file_map (&data_out)[FS_STAGE_MAX_MAPS])
    {
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + std::string("/maps.dat");
        filename = StringUtils::clean_filename(filename);
        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_all_maps - could not load file '" << filename << "'" << std::endl;
            return;
        }

        int fpos = stage_id * FS_STAGE_MAX_MAPS * sizeof(file_map);
        fp.seekg(fpos, std::ios::beg);

        for (int j=0; j<FS_STAGE_MAX_MAPS; j++) {
            fp.read(reinterpret_cast<char *>(&data_out[j]), sizeof(file_map));
        }

#ifdef WII
        wii_convert_map_data(data_out);
#endif


        fp.close();
    }

    bool file_io::file_exists(std::string filename) const
    {
        bool res = false;
        FILE *fp;
        fp = fopen(filename.c_str(), "rb");
        if (fp) {
            res = true;
            fclose(fp);
        }
        return res;
    }

    std::vector<std::string> file_io::read_game_list()
    {
        std::string filename = GAMEPATH + "/games/";
        filename = StringUtils::clean_filename(filename);
        return read_directory_list(filename, true);
    }

#ifdef PS2
    // @TODO
    /*
    int file_io::listcdvd(const char *path, entries *FileEntry) {
        static struct TocEntry TocEntryList[2048];
        char dir[1025];
        int i, n, t;

        strcpy(dir, &path[5]);
        // Directories first...

        CDVD_FlushCache();
        n = CDVD_GetDir(dir, NULL, CDVD_GET_DIRS_ONLY, TocEntryList, 2048, dir);

        strcpy(FileEntry[0].filename, "..");
        strcpy(FileEntry[0].displayname, "..");
             FileEntry[0].dircheck = 1;
        t = 1;

        for (i=0; i<n; i++) {
            if (TocEntryList[i].fileProperties & 0x02 && (!strcmp(TocEntryList[i].filename, ".") || !strcmp(TocEntryList[i].filename, ".."))) {
                continue; //Skip pseudopaths "." and ".."
            }
            FileEntry[t].dircheck = 1;
            strcpy(FileEntry[t].filename, TocEntryList[i].filename);
            strzncpy(FileEntry[t].displayname, FileEntry[t].filename, 63);
            t++;

            if (t >= 2046) {
                break;
            }
         }

        // Now files only

        CDVD_FlushCache();
        n = CDVD_GetDir(dir, NULL, CDVD_GET_FILES_ONLY, TocEntryList, 2048, dir);

        for (i=0; i<n; i++) {
            if (TocEntryList[i].fileProperties & 0x02 && (!strcmp(TocEntryList[i].filename, ".") || !strcmp(TocEntryList[i].filename, ".."))) {
                continue; //Skip pseudopaths "." and ".."
            }
            FileEntry[t].dircheck = 0;
            strcpy(FileEntry[t].filename, TocEntryList[i].filename);
            strzncpy(FileEntry[t].displayname, FileEntry[t].filename, 63);
            t++;
            if (t >= 2046) {
                break;
            }
        }
        return t;
    }
*/

    void file_io::ps2_listfiles(std::string filepath, std::vector<std::string> &res) {
        int n = 0;
        entries *FileEntry;
        iox_dirent_t buf;

        // force because code crashed //
        res.push_back("Rockbot2");
        return;

        printf(">>> file_io::ps2_listfiles::START <<<\n");

        int dd = fioDopen(filepath.c_str());
        if (dd < 0) {
            printf(">>> file_io::ps2_listfiles::CHECK #1 <<<\n");
            std::cout << "ps2_listfiles - Could not read directory" << std::endl;
            return;
        } else {
            printf("Directory opened!\n");
            printf(">>> file_io::ps2_listfiles::CHECK #2 <<<\n");

            /*
            //adds pseudo folder .. to every folder opened as mass: reported none but mc0: did
            strcpy(FileEntry[0].filename,"..");
            strcpy(FileEntry[0].displayname,"..");
            FileEntry[0].dircheck = 1;
            n=1;
            */

            printf(">>> file_io::ps2_listfiles::CHECK #3 <<<\n");

            while (fileXioDread(dd, &buf) > 0) {
                printf(">>> file_io::ps2_listfiles::CHECK #4 <<<\n");
                if (buf.stat.mode & FIO_S_IFDIR && (!strcmp(buf.name,".") || !strcmp(buf.name,".."))) {
                    printf(">>> file_io::ps2_listfiles::CHECK #5 <<<\n");
                    continue;
                }
                printf(">>> file_io::ps2_listfiles::CHECK #6 <<<\n");
                if (buf.stat.mode & FIO_S_IFDIR) {
                    printf(">>> file_io::ps2_listfiles::CHECK #7 <<<\n");
                    FileEntry[n].dircheck = 1;
                    strcpy(FileEntry[n].filename, buf.name);
                    res.push_back(std::string(FileEntry[n].filename));
                    n++;
                    printf(">>> file_io::ps2_listfiles::CHECK #8 <<<\n");
                }

                printf(">>> file_io::ps2_listfiles::CHECK #9 <<<\n");
                if(n > 2046) {
                    printf(">>> file_io::ps2_listfiles::CHECK #10 <<<\n");
                    break;
                }
            }
            if (dd >= 0) {
                printf(">>> file_io::ps2_listfiles::CHECK #11 <<<\n");
                fioDclose(dd);
                printf("Directory closed!\n");
            }
            printf(">>> file_io::ps2_listfiles::CHECK #12 <<<\n");
        }
    }
#endif

    // @TODO: make this work in multiplatform
    // http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
    std::vector<std::string> file_io::read_directory_list(std::string filename, bool dir_only)
    {
        std::vector<std::string> res;
        filename = StringUtils::clean_filename(filename);


        DIR *dir = opendir(filename.c_str());


#ifndef PLAYSTATION2
        struct dirent *entry = readdir(dir);

        while (entry != NULL) {

            //std::cout << ">>>>>>>>> entry->d_name: " << entry->d_name << std::endl;

            std::string dir_name = std::string(entry->d_name);
            if (dir_name != "." && dir_name != "..") {
                DIR *child_dir;
                std::string child_dir_path = filename + std::string("/") + dir_name;
                child_dir = opendir (child_dir_path.c_str());

                if (dir_only == true && child_dir != NULL) {
                    res.push_back(dir_name);
                } else if (dir_only == false && child_dir == NULL) {
                    res.push_back(dir_name);
                }
            }
            entry = readdir(dir);

        }
        closedir(dir);

#else
        res.push_back(std::string("Rockbot2"));
        /*
        if (filename.find("cdfs:") != std::string::npos) {
            // @TODO
            //ps2_listcdvd(filename, res);
        } else {
            ps2_listfiles(filename, res);
        }
        */
#endif
        return res;

    }



    std::vector<std::string> file_io::read_file_list(std::string filename)
    {
        return read_directory_list(filename, false);
    }





    bool file_io::save_exists()
    {
        std::string filename = std::string(SAVEPATH) + std::string("/") + GAMENAME + std::string(".sav");
        filename = StringUtils::clean_filename(filename);
#ifdef ANDROID
        // if config player services is set, and no save is found, get it from cloud
        if (game_config.android_use_play_services == true) {
            cloud_load_game(filename);
        }
#endif
        std::ifstream fp(filename.c_str());
        if (fp.good()) {
            return true;
        }

        if (GAMENAME == "Rockbot1") {
            // check for an old v1 format-save file
            std::string filename_v1 = std::string(SAVEPATH) + "/game_v301.sav";
            filename_v1 = StringUtils::clean_filename(filename_v1);
            FILE *v1_fp;
            v1_fp = fopen(filename_v1.c_str(), "rb");
            if (v1_fp) {
                // convert v1 save to v2 save
                CURRENT_FILE_FORMAT::st_save_v1 v1_save;
                std::cout << "########## filename_v1[" << filename_v1 << "] ############" << std::endl;
                int read_result = fread(&v1_save, sizeof(struct CURRENT_FILE_FORMAT::st_save_v1), 1, v1_fp);
                if (read_result  == -1) { // could not read v1 save
                    fclose(v1_fp);
                    return false;
                }
                std::cout << "[WRN] Converting v1 save to v2 format." << std::endl;
                if (v1_save.items.lifes > 9) {
                    v1_save.items.lifes = 3;
                }
                CURRENT_FILE_FORMAT::st_save v2_save;


                // ITEMS //
                v2_save.items.balancer = v1_save.items.balancer;
                v2_save.items.bolts = v1_save.items.bolts;
                v2_save.items.energy_saver = v1_save.items.energy_saver;
                v2_save.items.energy_tanks = v1_save.items.energy_tanks;
                v2_save.items.exit = v1_save.items.exit;
                if (v1_save.items.half_damage == 1) {
                    v2_save.items.half_damage = true;
                } else {
                    v2_save.items.half_damage = false;
                }
                v2_save.items.hyper_jump = v1_save.items.hyper_jump;
                v2_save.items.lifes = v1_save.items.lifes;
                v2_save.items.power_shot = v1_save.items.power_shot;
                if (v1_save.items.shock_guard == 1) {
                    v2_save.items.shock_guard = true;
                } else {
                    v2_save.items.shock_guard = false;
                }
                v2_save.items.special_tanks = v1_save.items.special_tanks;
                v2_save.items.speed_up = v1_save.items.speed_shot;
                if (v1_save.items.spike_guard == 1) {
                    v2_save.items.spike_guard = true;
                } else {
                    v2_save.items.spike_guard = false;
                }
                for (int i=0; i<WEAPON_COUNT; i++) {
                    v2_save.items.weapons[i] = v1_save.items.weapons[i];
                }
                v2_save.items.weapon_tanks = v1_save.items.weapon_tanks;



                v2_save.difficulty = v1_save.difficulty;
                v2_save.finished_stages = v1_save.finished_stages;
                v2_save.selected_player = v1_save.selected_player;
                for (int i=0; i<MAX_STAGES; i++) {
                    v2_save.stages[i] = v1_save.stages[i];
                }
                v2_save.used_countinue = v1_save.used_countinue;
                for (int i=0; i<FS_PLAYER_ARMOR_PIECES_MAX_V1; i++) {
                    v2_save.armor_pieces[i] = v1_save.armor_pieces[i];
                }
                v2_save.defeated_enemies_count = v1_save.defeated_enemies_count;

                fclose(v1_fp);

                write_save(v2_save);


                return true;
            }
        }
        return false;
    }

    bool file_io::can_access_castle(st_save &data_in)
    {
        for (int i=0; i<CASTLE1_STAGE1; i++) {
            if (data_in.stages[i] == 0) {
                return false;
            }
        }
        return true;
    }

    void file_io::load_config(format_v4::st_game_config& config)
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + CONFIG_FILENAME;
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "WARNING: Could not read config file '" << filename.c_str() << "'." << std::endl;
        } else {
            int read_result = fread(&config, sizeof(struct format_v4::st_game_config), 1, fp);
            if (read_result  == -1) {
                printf(">>file_io::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
                fflush(stdout);
                exit(-1);
            }
            fclose(fp);
        }
        if (config.get_current_platform() != config.platform) {
            config.reset();
        }
#ifndef PC
        config.video_filter = VIDEO_FILTER_NOSCALE;
        std::cout << "IO::load_config - SET video_filter to " << VIDEO_FILTER_NOSCALE << ", value: " << config.video_filter << std::endl;
#endif

        if (config.volume_music == 0) {
            config.volume_music = 128;
        }
        if (config.volume_sfx == 0) {
            config.volume_sfx = 128;
        }
    }

    void file_io::save_config(st_game_config &config) const
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + CONFIG_FILENAME;
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::cout << "Error: Could not open config file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fwrite(&config, sizeof(struct format_v4::st_game_config), 1, fp);
        fclose(fp);
    }

    void file_io::read_save(format_v4::st_save& data_out) const
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + std::string("/") + GAMENAME + std::string(".sav");
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "ERROR: Could not read save" << std::endl;
            exit(-1);
        }
        int read_result = fread(&data_out, sizeof(struct format_v4::st_save), 1, fp);
        if (read_result  == -1) {
            printf(">>file_io::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
            fflush(stdout);
            exit(-1);
        }


        if (GAME_FLAGS[FLAG_PLAYER1]) {
            data_out.selected_player = PLAYER_1;
        } else if (GAME_FLAGS[FLAG_PLAYER2]) {
            data_out.selected_player = PLAYER_2;
        } else if (GAME_FLAGS[FLAG_PLAYER3]) {
            data_out.selected_player = PLAYER_3;
        } else if (GAME_FLAGS[FLAG_PLAYER4]) {
            data_out.selected_player = PLAYER_4;
        }


        // ------- DEBUG ------- //
        /*
        data_out.stages[INTRO_STAGE] = 1;
        for (int i=STAGE1; i<=CASTLE1_STAGE5; i++) {
            data_out.stages[i] = 1;
        }
        //data_out.stages[INTRO_STAGE] = 1;
        //data_out.stages[STAGE5] = 1;
        //data_out.stages[STAGE3] = 0;
        data_out.armor_pieces[ARMOR_ARMS] = true;
        data_out.armor_pieces[ARMOR_BODY] = true;
        data_out.armor_pieces[ARMOR_LEGS] = true;
        */

        // ------- DEBUG ------- //


        if (data_out.items.lifes > 9) {
            data_out.items.lifes = 3;
        }
        if (data_out.items.weapon_tanks > 9) {
            data_out.items.weapon_tanks = 9;
        }
        if (data_out.items.energy_tanks > 9) {
            data_out.items.energy_tanks = 9;
        }
        if (data_out.items.special_tanks > 1) {
            data_out.items.special_tanks = 1;
        }


        fclose(fp);
    }

    bool file_io::write_save(format_v4::st_save& data_in)
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + std::string("/") + GAMENAME + std::string(".sav");
        filename = StringUtils::clean_filename(filename);
#ifdef ANDROID
        // if config is set to use cloud
        if (game_config.android_use_play_services == true) {
            cloud_save_game(filename);
        }
#endif
        fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::cout << "Error: Could not open save-file '" << filename << "'." << std::endl;
            return false;
        }
        fwrite(&data_in, sizeof(struct format_v4::st_save), 1, fp);
        fclose(fp);
        return true;
    }


    int file_io::read_stage_boss_id(Uint8 stage_n, file_stage &stages_data_out)
    {
        /// @NOTE: desabilitei essa função pois quando o formato de arquivo muda, causa um crash
        /// Isso provavelmente causará estouro de memória no PSP
        /// Então estou sobrescrevendo o próprio stage_data, passado como referência
        FILE *fp;

        std::string filename = std::string(FILEPATH) + "/stages" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);

        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            fflush(stdout);
            exit(-1);
        }
        fseek(fp, sizeof(format_v4::file_stage) * stage_n, SEEK_SET);
        int read_result = fread(&stages_data_out, sizeof(struct format_v4::file_stage), 1, fp);
        if (read_result == -1) {
            printf(">>file_io::read_game - Error reading struct data from stage file.\n");
            fflush(stdout);
            exit(-1);
        }
        fclose(fp);
        return stages_data_out.boss.id_npc;
    }




    // create and save empty files for new-game
    void file_io::generate_files()
    {
        CURRENT_FILE_FORMAT::file_game data_in;
        write_game(data_in);
        CURRENT_FILE_FORMAT::file_stages stages_data_in;
        write_all_stages(stages_data_in);

        CURRENT_FILE_FORMAT::file_map maps_data_in[FS_MAX_STAGES][FS_STAGE_MAX_MAPS];
        write_all_maps(maps_data_in);
    }

    int file_io::get_heart_pieces_number(CURRENT_FILE_FORMAT::st_save game_save)
    {
        int res = PLAYER_INITIAL_HP;
        for (int i=0; i<WEAPON_COUNT; i++) {
            if (game_save.items.heart_pieces[i] == true) {
                res++;
            }
        }
        return res;
    }

    void file_io::read_castle_data(file_castle &data_out)
    {
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + "/castle_data" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        fp.read(reinterpret_cast<char *>(&data_out), sizeof(struct format_v4::file_castle));
        fp.close();
    }

    void file_io::write_castle_data(file_castle &data_in)
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "/castle_data" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_all_stages - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        } else {
            std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
        }
        fp.write(reinterpret_cast<char *>(&data_in), sizeof(struct format_v4::file_castle));
        fp.close();
    }

    void file_io::read_stage_select_data(CURRENT_FILE_FORMAT::file_stage_select &data_out, bool check_error)
    {
        std::string filename = std::string(FILEPATH) + "/stage_select_data" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        FILE *fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            fflush(stdout);
            return;
        }
        int read_result = fread(&data_out, sizeof(struct CURRENT_FILE_FORMAT::file_stage_select), 1, fp);


#ifdef ANDROID
        std::string log_line_filename = "filename[" + filename + "]";
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", log_line_filename.c_str());
#endif

        fclose(fp);

    }

    void file_io::write_stage_select_data(CURRENT_FILE_FORMAT::file_stage_select &data_in)
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "/stage_select_data" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_all_stages - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        } else {
            std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
        }
        fp.write(reinterpret_cast<char *>(&data_in), sizeof(struct CURRENT_FILE_FORMAT::file_stage_select));
        fp.close();
    }


#ifdef WII
    void file_io::wii_convert_game_data(file_game &data_out)
    {
        for (int i=0; i<MAX_WEAPON_N; i++) {
            sint16_to_little_endian(data_out.weapon_menu_colors[i].r);
            sint16_to_little_endian(data_out.weapon_menu_colors[i].g);
            sint16_to_little_endian(data_out.weapon_menu_colors[i].b);
        }
        for (int i=0; i<FS_PLAYER_ARMOR_PIECES_MAX; i++) {
            for (int j=0; j<FS_MAX_PLAYERS; j++) {
                int_to_little_endian(data_out.armor_pieces[i].special_ability[j]);
                for (int k=0; k<FS_DIALOG_LINES; k++) {
                    int_to_little_endian(data_out.armor_pieces[i].got_message[j][k]);
                }
            }
        }
    }


    void file_io::wii_convert_map_data(file_map (&data_out)[FS_STAGE_MAX_MAPS])
    {
        for (int i=0; i<FS_STAGE_MAX_MAPS; i++) {
            sint16_to_little_endian(data_out[i].backgrounds[0].adjust_y);
            sint16_to_little_endian(data_out[i].backgrounds[1].adjust_y);
            sint16_to_little_endian(data_out[i].background_color.r);
            sint16_to_little_endian(data_out[i].background_color.g);
            sint16_to_little_endian(data_out[i].background_color.b);

            for (int j=0; j<FS_MAX_MAP_NPCS; j++) {
                sint16_to_little_endian(data_out[i].map_npcs[j].start_point.x);
                sint16_to_little_endian(data_out[i].map_npcs[j].start_point.y);
            }
            for (int j=0; j<FS_MAX_MAP_OBJECTS; j++) {
                sint16_to_little_endian(data_out[i].map_objects[j].link_dest.x);
                sint16_to_little_endian(data_out[i].map_objects[j].link_dest.y);
                sint16_to_little_endian(data_out[i].map_objects[j].start_point.x);
                sint16_to_little_endian(data_out[i].map_objects[j].start_point.y);
            }
        }
    }
#endif
}

