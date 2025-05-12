#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL3/SDL.h>

#ifdef __linux__
#include <signal.h>
#else
#include <windows.h>
#define sleep(s) Sleep(s * 1000)
#endif

#include <map>
#include <thread>

#ifdef __linux__
void sig_int(int signo){
	exit(0);
}
#endif

FILE *log_file = NULL;
#ifdef __linux__
#define LOG_FILE_NAME "sdl_joystick_dumper_linux.txt"
#else
#define LOG_FILE_NAME "sdl_joystick_dumper_windows.txt"
#endif

#define LOG(...){ \
	printf(__VA_ARGS__); \
	if(log_file == NULL) \
		log_file = fopen(LOG_FILE_NAME, "w"); \
	if(log_file != NULL){ \
		fprintf(log_file, __VA_ARGS__); \
		fflush(log_file); \
	} \
}

int SDLCALL autocenter_disable_tick(void *){
	while(true){
		SDL_Event user_event = {0};
		user_event.type = SDL_EVENT_USER;
		user_event.user.code = 1;
		SDL_PushEvent(&user_event);
		sleep(1);
	}
}

int main(){
	SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);
	std::map<SDL_JoystickID, SDL_Joystick *> joysticks;
	std::map<SDL_Joystick *, SDL_Haptic *> haptic_devices;

	#ifdef __linux__
	struct sigaction act = {0};
	act.sa_handler = sig_int;
	sigaction(SIGINT, &act, NULL);
	#endif

	SDL_Thread *autocenter_disable_tick_thread = SDL_CreateThread(autocenter_disable_tick, "autocenter disabling timer", NULL);
	if(autocenter_disable_tick_thread == NULL){
		LOG("failed creating auto center disabling tick, %s\n", SDL_GetError());
		exit(1);
	}

	while(true){
		SDL_Event event;
		SDL_WaitEvent(&event);
		switch(event.type)
		{
			case SDL_EVENT_USER:{
				for(auto haptic_device : haptic_devices){
					if(!SDL_SetHapticAutocenter(haptic_device.second, 0)){
						//LOG("failed disabling autocenter, %s\n", SDL_GetError());
					}
				}
			}
			case SDL_EVENT_JOYSTICK_ADDED:{
				SDL_JoyDeviceEvent *e = (SDL_JoyDeviceEvent *)&event;
				SDL_Joystick *handle = SDL_OpenJoystick(e->which);
				if (handle != nullptr){
					SDL_PropertiesID prop_id = SDL_GetJoystickProperties(handle);
					bool led = SDL_GetBooleanProperty(prop_id, SDL_PROP_JOYSTICK_CAP_MONO_LED_BOOLEAN, false);
					bool color_led = SDL_GetBooleanProperty(prop_id, SDL_PROP_JOYSTICK_CAP_RGB_LED_BOOLEAN, false);
					joysticks[e->which] = handle;
					LOG("joystick %u %p %04x:%04x added, buttons: %d, hats: %d, axis: %d, led: %s, color led: %s\n", e->which, handle, SDL_GetJoystickVendor(handle), SDL_GetJoystickProduct(handle), SDL_GetNumJoystickButtons(handle), SDL_GetNumJoystickHats(handle), SDL_GetNumJoystickAxes(handle), led ? "true" : "false", color_led ? "true" : "false");

					SDL_Haptic *haptic_handle = SDL_OpenHapticFromJoystick(handle);
					if(haptic_handle != NULL){
						haptic_devices[handle] = haptic_handle;
						int max_num_effects = SDL_GetMaxHapticEffects(haptic_handle);
						int max_num_effects_playing = SDL_GetMaxHapticEffectsPlaying(haptic_handle);
						uint32_t haptic_features = SDL_GetHapticFeatures(haptic_handle);
						char feature_buf[4096] = {0};
						int feature_buf_offset = 0;
						#define CHECK_FEATURE(val, name){ \
							if(haptic_features & val) \
								feature_buf_offset += sprintf(&feature_buf[feature_buf_offset], "%s ", name); \
						}
						CHECK_FEATURE(SDL_HAPTIC_CONSTANT, "constant");
						CHECK_FEATURE(SDL_HAPTIC_SINE, "sine");
						CHECK_FEATURE(SDL_HAPTIC_SQUARE, "square");
						CHECK_FEATURE(SDL_HAPTIC_TRIANGLE, "triangle");
						CHECK_FEATURE(SDL_HAPTIC_SAWTOOTHUP, "sawtoothup");
						CHECK_FEATURE(SDL_HAPTIC_SAWTOOTHDOWN, "sawtoothdown");
						CHECK_FEATURE(SDL_HAPTIC_RAMP, "ramp");
						CHECK_FEATURE(SDL_HAPTIC_SPRING, "spring");
						CHECK_FEATURE(SDL_HAPTIC_DAMPER, "damper");
						CHECK_FEATURE(SDL_HAPTIC_INERTIA, "inertia");
						CHECK_FEATURE(SDL_HAPTIC_FRICTION, "friction");
						CHECK_FEATURE(SDL_HAPTIC_LEFTRIGHT, "leftright");
						CHECK_FEATURE(SDL_HAPTIC_CUSTOM, "custom");
						CHECK_FEATURE(SDL_HAPTIC_GAIN, "gain");
						CHECK_FEATURE(SDL_HAPTIC_AUTOCENTER, "autocenter");
						CHECK_FEATURE(SDL_HAPTIC_STATUS, "status");
						CHECK_FEATURE(SDL_HAPTIC_PAUSE, "pause");
						#undef CHECK_FEATURE
						if(SDL_HapticRumbleSupported(haptic_handle)){
							feature_buf_offset += sprintf(&feature_buf[feature_buf_offset], "%s ", "rumble");
						}

						SDL_SetHapticAutocenter(haptic_handle, 0);

						LOG("haptic device %p added from %p, max num effects: %d, max num effects playing: %d, features: %s\n", haptic_handle, handle, max_num_effects, max_num_effects_playing, feature_buf);
					}
				}
				break;
			}
			case SDL_EVENT_JOYSTICK_REMOVED:{
				SDL_JoyDeviceEvent *e = (SDL_JoyDeviceEvent *)&event;
				auto joystick_entry = joysticks.find(e->which);
				if(joystick_entry != joysticks.end()){
					SDL_Joystick *joystick = joystick_entry->second;
					LOG("joystick %d %04x:%04x removed\n", SDL_GetJoystickID(joystick), SDL_GetJoystickVendor(joystick), SDL_GetJoystickProduct(joystick));
					auto haptic_entry = haptic_devices.find(joystick);
					if(haptic_entry != haptic_devices.end()){
						SDL_CloseHaptic(haptic_entry->second);
					}
					SDL_CloseJoystick(joystick);
				}
				SDL_Joystick *joystick = joysticks[e->which];
				break;
			}
			case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
			case SDL_EVENT_JOYSTICK_BUTTON_UP:{
				SDL_JoyButtonEvent *e = (SDL_JoyButtonEvent *)&event;
				uint16_t vendor = SDL_GetJoystickVendorForID(e->which);
				uint16_t product = SDL_GetJoystickProductForID(e->which);
				if (vendor != 0){
					LOG("%04x:%04x button %u %s\n", vendor, product, e->button, e->down ? "down" : "up");
				}
				break;
			}
			case SDL_EVENT_JOYSTICK_HAT_MOTION:{
				SDL_JoyHatEvent *e = (SDL_JoyHatEvent *)&event;
				uint16_t vendor = SDL_GetJoystickVendorForID(e->which);
				uint16_t product = SDL_GetJoystickProductForID(e->which);
				LOG("%04x:%04x hat %u %u\n", vendor, product, e->hat, e->value);
				break;
			}
			case SDL_EVENT_JOYSTICK_AXIS_MOTION:{
				SDL_JoyAxisEvent *e = (SDL_JoyAxisEvent *)&event;
				uint16_t vendor = SDL_GetJoystickVendorForID(e->which);
				uint16_t product = SDL_GetJoystickProductForID(e->which);
				LOG("%04x:%04x axis %u %d\n", vendor, product, e->axis, e->value);
				break;
			}
		}
	}
}
