#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_flash.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

static const char *TAG = "FAT";

// Handle of the wear levelling library instance
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

// Mount path for the partition
const char *base_path = "/fatfs_image";

// Mount FATFS partition
static bool mount_fatfs(const char* partition_label)
{
    ESP_LOGI(TAG, "Mounting FAT filesystem");
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(base_path, partition_label, &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return false;
    }
    return true;
}

void app_main(void)
{
  	const char *partition_label = "storage";
    // Initialize FAT FS in the partition
    if (!mount_fatfs(partition_label)) {
        return;
    }

    // Print FAT FS size information
    uint64_t bytes_total, bytes_free;
    esp_vfs_fat_info(base_path, &bytes_total, &bytes_free);
    ESP_LOGI(TAG, "FAT FS: %" PRIu64 " kB total, %" PRIu64 " kB free", bytes_total / 1024, bytes_free / 1024);

	// 读取写入的镜像文件
    // Open file for reading
    ESP_LOGI(TAG, "Reading file");
    FILE *f = fopen("/fatfs_image/hello.txt", "rb");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[128];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);
	// Unmount FAT FS
	ESP_LOGI(TAG, "Unmounting FAT filesystem");
	esp_err_t unmount_err = esp_vfs_fat_spiflash_unmount_rw_wl(base_path, s_wl_handle);
	if (unmount_err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to unmount FATFS (%s)", esp_err_to_name(unmount_err));
		return;
	}
}


