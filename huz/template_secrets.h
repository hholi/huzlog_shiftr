#define MAX_SSID 4

const char * ssids[MAX_SSID][5] = {
		/* SSID,  PWD, EAP_USER, mode EAP or WAP, Active Y or N */
		{"ssid1", "pw1", "USER", "EAP", "Y"},
		{"ssid2", "pw2", "", "WAP", "Y"}
};


#define MQTT_USER "user"
#define MQTT_TOKEN "pw"
