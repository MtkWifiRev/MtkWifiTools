#!/usr/bin/python

# Made by Edoardo Mantovani, 20 December 2024
# Simple program which permits to scan, download and analyze a vast amount of files present in the various dumps from the dumps.tadiphone.dev Gitlab repository

import 	time
import  requests
import  sys
import 	os
import  re
import  yaml
import  argparse

from 	selenium 			import webdriver
from 	selenium.webdriver.common.by 	import By
from 	webdriver_manager.chrome 	import ChromeDriverManager

### set up the args

parser 					= argparse.ArgumentParser()
parser.add_argument("page_start", help="define the start page for the https://dumps.tadiphone.dev/dumps page", type=int)

### colors definition:

class style():
    BLACK = '\033[30m'
    RED = '\033[31m'
    GREEN = '\033[32m'
    YELLOW = '\033[33m'
    BLUE = '\033[34m'
    MAGENTA = '\033[35m'
    CYAN = '\033[36m'
    WHITE = '\033[37m'
    UNDERLINE = '\033[4m'
    RESET = '\033[0m'

### global variables definition:

target_vendor_parser			= 'vendor/Mediatek/'
target_vendor_folder 			= 'vendor'
target_fw_folder			= 'firmware'
target_fw_files				= [
						'WIFI_RAM_CODE_',
						'soc0_',
						'soc3_',
						'soc5_',
						'soc7_',
						'patch_mcu'
					]
target_prop_file			= '.prop'
target_prop_strings			= [
						'ro.vendor.build.date',
						'ro.board.platform',
						'persist.sys.timezone',
						'ro.mediatek.chip_ver',
						'ro.wlan.gen',
						### legacy
						'ro.product.vendor.device',
						'ro.vendor.build.version.sdk',
						'ro.vendor.wlan.gen',
						'ro.vendor.bt.platform'
						### vendor legacy
						'ro.product.vendor.model'
					]

target_output_folder			= './output'


page_vendor_not_available		= 'No subgroups or projects.'
page_vendor_number_counter		= 1

### global stats declaration:

scraped_firmware_count 			= 0
scraped_firmware_names			= []

scraped_phone_current_brand		= None
scraped_phone_current_model		= None

### start of 'vendor_scrape_blob_data'
def vendor_scrape_blob_data(vendor_phone_metadata_file):
	if vendor_phone_metadata_file is None:
		return

	try:
		driver.get(vendor_phone_metadata_file)
	except:
		return

	time.sleep(2)
	scraped_prop_file		= driver.page_source
	for prop_element in target_prop_strings:
		if prop_element in scraped_prop_file:
			pattern 		= rf'{re.escape(prop_element)}\=([^\s]*)'
			match 			= re.search(pattern, string=scraped_prop_file)
			if match is not None:
				print(prop_element + " : " + match)


### end of 'vendor_scrape_blob_data'

### start of 'vendor_get_additional_data'
def vendor_get_additional_data(fw_download_link):
	if fw_download_link is None:
		return
	vendor_path			= fw_download_link.split("firmware")[0]
	vendor_path			= vendor_path.replace("/raw/", "/blob/")
	try:
		driver.get(vendor_path)
		# fix: use '3' seconds instead of '1' as a waiting time due to the high number of possible blobs present in certain dumps
		# should fix the crash when trying to scrape 'asus/asus_ai2205'
		time.sleep(2)
		links                           	= driver.find_elements(By.TAG_NAME, "a")
		vendor_phone_metadata_scraped_links   	= [
                		                            	link.get_attribute("href") for link in links if url in
                                                        	link.get_attribute("href") and "#" not in
                                                        	link.get_attribute("href") and url !=
                                                        	link.get_attribute("href")
                                                	]

		for vendor_phone_metadata_file in vendor_phone_metadata_scraped_links:
			if target_prop_file in vendor_phone_metadata_file:
				vendor_scrape_blob_data(vendor_phone_metadata_file)
	except:
                return

### end of 'vendor_get_additional_data'

### start of 'vendor_acquire_data'
def vendor_acquire_data(fw_download_link, fw_final_path_in_fs):
	if fw_download_link is None or fw_final_path_in_fs is None:
		return

	vendor_get_additional_data(fw_download_link)
### end of 'vendor_acquire_data'

### start of 'vendor_download_fw_blob'
def vendor_download_fw_blob(vendor_phone_fw_file):
	global scraped_phone_current_brand
	global scraped_phone_current_model
	global scraped_firmware_count

	if vendor_phone_fw_file is None:
		return

	fw_download_link 		= vendor_phone_fw_file.replace("/blob/", "/raw/")
	scraped_firmware_count		= scraped_firmware_count + 1
	fw_partial_name			= (fw_download_link.split("/")[-1])
	fw_final_name			= (fw_partial_name.split("?")[0])
	fw_final_path_name		= target_output_folder   + "/" + scraped_phone_current_brand + "/" + scraped_phone_current_model
	fw_final_path_in_fs		= fw_final_path_name     + "/" + fw_final_name
	fw_downloaded_data              = requests.get(fw_download_link, allow_redirects=False)

	print(	"firmware " 		+
		style.MAGENTA 		+
		fw_final_name 		+
		style.RESET 		+
		" found: under " 	+
		style.GREEN		+ scraped_phone_current_brand + "/" + scraped_phone_current_model + style.RESET
	)
        ### create the folder under ./output and then place the firmware

	os.makedirs(fw_final_path_name, exist_ok=True)
	open(fw_final_path_in_fs, 'wb').write(fw_downloaded_data.content)

	### once we get the file in the FS, we can starting with the gathering of additional informations like:
	### - header of the firmware, additional metadata
	### - if the firmware is in plaintext or not and which versions is using
	### - SoC used by the smartphone
	### - driver gen and the version of the driver too

	vendor_acquire_data(fw_download_link, fw_final_path_in_fs)
	return
### end of 'vendor_download_fw_blob'

### start of 'vendor_acquire_fw_blob'
def vendor_acquire_fw_blob(vendor_phone_fw_folder):
	if vendor_phone_fw_folder is None:
		return

	try:
		driver.get(vendor_phone_fw_folder)
		# fix: use '3' seconds instead of '1' as a waiting time due to the high number of possible blobs present in certain dumps
		# should fix the crash when trying to scrape 'asus/asus_ai2205'
		time.sleep(3)
		links				= driver.find_elements(By.TAG_NAME, "a")
		vendor_phone_fw_scraped_links   = [
                	                                link.get_attribute("href") for link in links if url in
                        	                        link.get_attribute("href") and "#" not in
                                	                link.get_attribute("href") and url !=
                                        	        link.get_attribute("href")
                                        	]

		for vendor_phone_fw_file in vendor_phone_fw_scraped_links:
			for target_fw_file in target_fw_files:
				if target_fw_file in vendor_phone_fw_file:
					vendor_download_fw_blob(vendor_phone_fw_file)
	except:
		return

	return
### end of 'vendor_acquire_fw_blob'

### start of 'vendor_acquire_fw_folder'
def vendor_acquire_fw_folder(vendor_phone_fs_folder):
	if vendor_phone_fs_folder is None:
		return

	try:
		driver.get(vendor_phone_fs_folder)
		time.sleep(2)
		links                           = driver.find_elements(By.TAG_NAME, "a")
		vendor_phone_fs_scraped_links   = [
                                	                link.get_attribute("href") for link in links if url in
                        	 	                link.get_attribute("href") and "#" not in
                                        	        link.get_attribute("href") and url !=
                                        	        link.get_attribute("href")
                                        	]
	except:
		return

	if vendor_phone_fs_scraped_links is not None:
		for vendor_phone_fw_folder in vendor_phone_fs_scraped_links:
			if target_fw_folder in vendor_phone_fw_folder:
				vendor_acquire_fw_blob(vendor_phone_fw_folder)
	return
### end of 'vendor_acquire_fw_folder'

### start of 'vendor_process_folder'
def vendor_process_folder(vendor_specific_phone_link):
	global scraped_phone_current_model

	if vendor_specific_phone_link is None:
		return

	try:
		driver.get(vendor_specific_phone_link)
		time.sleep(1)
		links				= driver.find_elements(By.TAG_NAME, "a")
		vendor_phone_scraped_link_urls  = [
                	                               link.get_attribute("href") for link in links if url in
        	                                       link.get_attribute("href") and "#" not in
                	                               link.get_attribute("href") and url !=
                        	                       link.get_attribute("href")
						]
	except:
		return

	if vendor_phone_scraped_link_urls is not None:
		scraped_phone_current_model	= vendor_specific_phone_link.split("/")[-1]
		for vendor_phone_fs_folder in vendor_phone_scraped_link_urls:
			if target_vendor_folder in vendor_phone_fs_folder:
				vendor_acquire_fw_folder(vendor_phone_fs_folder)

	return
### end of 'vendor_process_folder'

### start of 'vendor_parse_link'
def vendor_parse_link(url):
	global scraped_phone_current_brand
	global page_vendor_number_counter

	if url is None:
		return

	url_with_page				= url + "?page=" + str(page_vendor_number_counter)

	try:
		driver.get(url_with_page)
		time.sleep(3)
		links	 			= driver.find_elements(By.TAG_NAME, "a")

		vendor_scraped_link_urls	= [
							link.get_attribute("href") for link in links if url in
							link.get_attribute("href") and "#" not in
							link.get_attribute("href") and url !=
							link.get_attribute("href")
						]
		if len(vendor_scraped_link_urls) == 0:
			return None
	except:
		return None

	vendor_scraped_unique_urls     		= list(set(vendor_scraped_link_urls))


	scraped_phone_current_brand		= url.split("/")[-1]
	for vendor_specific_phone_link in vendor_scraped_unique_urls:
		print("working on " + vendor_specific_phone_link)
		vendor_process_folder(vendor_specific_phone_link)

	### check if there is a page 2,3,4 ecc.. for the vendor

	page_vendor_number_counter		= page_vendor_number_counter + 1
	if vendor_parse_link(url) == None:
		page_vendor_number_counter	= 1
		return
#end of 'vendor_parse_link'



##########################################################
# start of the real program				 #
##########################################################

### print the logo

logo = """
  █████▒█     █░    ███▄ ▄███▓ ▄▄▄        ██████   ██████    ▓█████▄  ▒█████   █     █░███▄    █  ██▓     ▒█████   ▄▄▄      ▓█████▄ ▓█████  ██▀███  
▓██   ▒▓█░ █ ░█░   ▓██▒▀█▀ ██▒▒████▄    ▒██    ▒ ▒██    ▒    ▒██▀ ██▌▒██▒  ██▒▓█░ █ ░█░██ ▀█   █ ▓██▒    ▒██▒  ██▒▒████▄    ▒██▀ ██▌▓█   ▀ ▓██ ▒ ██▒
▒████ ░▒█░ █ ░█    ▓██    ▓██░▒██  ▀█▄  ░ ▓██▄   ░ ▓██▄      ░██   █▌▒██░  ██▒▒█░ █ ░█▓██  ▀█ ██▒▒██░    ▒██░  ██▒▒██  ▀█▄  ░██   █▌▒███   ▓██ ░▄█ ▒
░▓█▒  ░░█░ █ ░█    ▒██    ▒██ ░██▄▄▄▄██   ▒   ██▒  ▒   ██▒   ░▓█▄   ▌▒██   ██░░█░ █ ░█▓██▒  ▐▌██▒▒██░    ▒██   ██░░██▄▄▄▄██ ░▓█▄   ▌▒▓█  ▄ ▒██▀▀█▄  
░▒█░   ░░██▒██▓    ▒██▒   ░██▒ ▓█   ▓██▒▒██████▒▒▒██████▒▒   ░▒████▓ ░ ████▓▒░░░██▒██▓▒██░   ▓██░░██████▒░ ████▓▒░ ▓█   ▓██▒░▒████▓ ░▒████▒░██▓ ▒██▒
 ▒ ░   ░ ▓░▒ ▒     ░ ▒░   ░  ░ ▒▒   ▓▒█░▒ ▒▓▒ ▒ ░▒ ▒▓▒ ▒ ░    ▒▒▓  ▒ ░ ▒░▒░▒░ ░ ▓░▒ ▒ ░ ▒░   ▒ ▒ ░ ▒░▓  ░░ ▒░▒░▒░  ▒▒   ▓▒█░ ▒▒▓  ▒ ░░ ▒░ ░░ ▒▓ ░▒▓░
 ░       ▒ ░ ░     ░  ░      ░  ▒   ▒▒ ░░ ░▒  ░ ░░ ░▒  ░ ░    ░ ▒  ▒   ░ ▒ ▒░   ▒ ░ ░ ░ ░░   ░ ▒░░ ░ ▒  ░  ░ ▒ ▒░   ▒   ▒▒ ░ ░ ▒  ▒  ░ ░  ░  ░▒ ░ ▒░
 ░ ░     ░   ░     ░      ░     ░   ▒   ░  ░  ░  ░  ░  ░      ░ ░  ░ ░ ░ ░ ▒    ░   ░    ░   ░ ░   ░ ░   ░ ░ ░ ▒    ░   ▒    ░ ░  ░    ░     ░░   ░ 
           ░              ░         ░  ░      ░        ░        ░        ░ ░      ░            ░     ░  ░    ░ ░        ░  ░   ░       ░  ░   ░    
"""

print("\033c", end="")

print(style.RED)
print(logo)
print(style.RESET)

options                 	= webdriver.ChromeOptions()

options.add_argument("--headless=new")
options.add_argument("--ignore-certificate-errors")
options.add_argument("--disable-popup-blocking")
options.add_argument("--incognito")


driver 				= webdriver.Chrome(options=options)

if driver is None:
	print("Impossible to initialize the Chrome webdriver")
	exit(-1)

if os.path.exists(target_output_folder) == False:
	os.mkdir(target_output_folder)

android_fw_counter 		= 3
android_fw_url 	   		= "https://dumps.tadiphone.dev/dumps?page="

android_fw_to_aquire_url 	= android_fw_url + str(android_fw_counter)

while android_fw_counter > 0:
	driver.get(android_fw_to_aquire_url)

	if driver.current_url is None:
		print("Error with the Chrome webdriver!")
		break
	else:
		print("========================================")
		print(driver.current_url)
		print("========================================")

	time.sleep(1)

	links 			= driver.find_elements(By.TAG_NAME, "a")

	if links is not None:
		link_urls 		= [
						link.get_attribute("href") for link in links if "dumps/" in
						link.get_attribute("href") and "#" not in
						link.get_attribute("href") and "/-/" not in
						link.get_attribute("href")
					]

		if link_urls is not None:
			unique_urls 		= list(set(link_urls))

		if unique_urls is not None:
			for url in unique_urls:
				if url is not None:
					driver.get(url)
					time.sleep(1)
					vendor_parse_link(url)
					### reset the page counter
					page_vendor_number_counter = 1

	android_fw_counter 		= android_fw_counter + 1
	android_fw_to_aquire_url        = android_fw_url + str(android_fw_counter)


print("final stats:")
print("final number of aquired firmware: " + str(scraped_firmware_count))

driver.quit()
