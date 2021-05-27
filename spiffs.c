/*
 * MIT License
 *
 * Copyright (c) 2021 wolffshots
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file spiffs.c
 * @brief implementations for component spiffs
 */

// rest of the includes
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include <sys/param.h>
#include <sys/unistd.h>
#include <dirent.h>

#include "spiffs.h"

#include "utility.h"

// variables
static const char *TAG = CONFIG_SPIFFS_TAG;

// function definitions

off_t get_file_size(const char *filename)
{
    struct stat st;
    if (stat(filename, &st) == 0)
    {
        ESP_LOGI(TAG, "file size: %ld", st.st_size);
        return st.st_size;
    }
    ESP_LOGI(TAG, "Cannot determine size of %s\n",
             filename);
    return -1;
}

void read_file(const char *pwd)
{
    ESP_LOGI(TAG, "Reading %s", pwd);
    // Open for reading hello.txt
    FILE *file = fopen(pwd, "r");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open %s", pwd);
        return;
    }
    // print_chip_info();
    off_t file_size = get_file_size(pwd);
    char buf[file_size < 2000 ? file_size : 2000];
    // print_chip_info();
    off_t current_position = 2000;
    while (current_position < file_size + 2000)
    {
        memset(buf, 0, sizeof(buf));
        fread(buf, 1, sizeof(buf), file);
        ESP_LOGI(TAG, "%s", buf);
        current_position += 1088;
    }
    fclose(file);
    // Display the read contents from the file
    ESP_LOGI(TAG, "Read from %s", pwd);
}

void list_files()
{
    /* Iterate over all files / folders and fetch their names and sizes */
    char entrypath[40];
    const char *entrytype;
    struct dirent *entry;
    DIR *dir = opendir("/spiffs");
    const size_t dirpath_len = strlen("/spiffs");
    while ((entry = readdir(dir)) != NULL)
    {
        entrytype = (entry->d_type == DT_DIR ? "directory" : "file");
        strlcpy(entrypath + dirpath_len, entry->d_name, sizeof(entrypath) - dirpath_len);
        ESP_LOGI(TAG, "%s : %s", entrytype, entry->d_name);
    }
    closedir(dir);
}
/**
 * an example function
 */
void spiffs_init()
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    list_files();

    // Read and display the contents of a small text file (hello.txt)
    read_file("/spiffs/LICENSE.md");

    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "SPIFFS unmounted");
}
