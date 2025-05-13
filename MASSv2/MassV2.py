#!/usr/bin/python

# Made by Edoardo Mantovani, 20 December 2025
# better version of MASS, permits to mass download both the firmware and userspace libraries for discovering if ICAP is enabled or not.

import  time
import  requests
import  sys
import 	os
import  re
import  yaml
import  argparse

from selenium 				import webdriver
from selenium.webdriver.common.by 	import By
from webdriver_manager.chrome 		import ChromeDriverManager
from selenium.webdriver.chrome.service  import Service

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
target_vendor_folder 			= 'vendor?ref_type=heads'

target_output_folder			= './output'


page_vendor_not_available		= 'No subgroups or projects.'
page_vendor_number_counter		= 1

### global stats declaration:

scraped_firmware_count 			= 0
scraped_firmware_names			= []

scraped_phone_current_brand		= None
scraped_phone_current_model		= None


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
				vendor_phone_fs_folder = '/lib64/libwifitest.so' . join(vendor_phone_fs_folder.split("?ref_type=heads"))
				vendor_phone_fs_folder = vendor_phone_fs_folder.replace("/tree/", "/raw/")
				fw_final_path_in_fs		= './output/' + scraped_phone_current_model
				fw_downloaded_data              = requests.get(vendor_phone_fs_folder, allow_redirects=False)
				if( "DOCTYPE html" not in str(fw_downloaded_data.content[0:24]) ):
					print("saving libwifitest.so under " + fw_final_path_in_fs)
        				### create the folder under ./output and then place the firmware
					os.makedirs(fw_final_path_in_fs, exist_ok=True)
					fw_final_path_in_fs	       += '/libwifitest.so'
					open(fw_final_path_in_fs, 'wb').write(fw_downloaded_data.content)
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
		vendor_process_folder(vendor_specific_phone_link)


	### check if there is a page 2,3,4 ecc.. for the vendor

	page_vendor_number_counter		= page_vendor_number_counter + 1
	if vendor_parse_link(url) == None:
		page_vendor_number_counter	= 1
		return
#end of 'vendor_parse_link'



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


try:
	driver 				= webdriver.Chrome(options=options)
except:
	Service				= Service(executable_path="/usr/bin/chromedriver")
	driver 				= webdriver.Chrome(service=Service, options=options)

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
