#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <SDL3/SDL.h>

#include <vector>

void sig_int(int signo){
	exit(0);
}

int main(){
	SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);

	std::vector<SDL_Joystick *> joysticks;

	struct sigaction act = {0};
	act.sa_handler = sig_int;
	sigaction(SIGINT, &act, NULL);

	while(true){
		SDL_Event event;
		SDL_WaitEvent(&event);
		switch(event.type)
		{
			case SDL_EVENT_JOYSTICK_ADDED:{
				SDL_JoyDeviceEvent *e = (SDL_JoyDeviceEvent *)&event;
				SDL_Joystick *handle = SDL_OpenJoystick(e->which);
				if (handle != nullptr){
					SDL_PropertiesID prop_id = SDL_GetJoystickProperties(handle);
					bool led = SDL_GetBooleanProperty(prop_id, SDL_PROP_JOYSTICK_CAP_MONO_LED_BOOLEAN, false);
					bool color_led = SDL_GetBooleanProperty(prop_id, SDL_PROP_JOYSTICK_CAP_RGB_LED_BOOLEAN, false);
					printf("joystick %d %04x:%04x added, buttons %d, hats %d, axis %d, led %s, color led %s\n", e->which, SDL_GetJoystickVendor(handle), SDL_GetJoystickProduct(handle), SDL_GetNumJoystickButtons(handle), SDL_GetNumJoystickHats(handle), SDL_GetNumJoystickAxes(handle), led ? "true" : "false", color_led ? "true" : "false");

					joysticks.push_back(handle);
				}				
			}
			case SDL_EVENT_JOYSTICK_REMOVED:
			{
				for(auto joystick = joysticks.begin();joystick != joysticks.end();joystick++){
					if(!SDL_JoystickConnected(*joystick)){
						printf("joystick %d %04x:%04x removed", SDL_GetJoystickID(*joystick), SDL_GetJoystickVendor(*joystick), SDL_GetJoystickProduct(*joystick));
						SDL_CloseJoystick(*joystick);
					}
				}
			}
			case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
			case SDL_EVENT_JOYSTICK_BUTTON_UP:{
				SDL_JoyButtonEvent *e = (SDL_JoyButtonEvent *)&event;
				uint16_t vendor = SDL_GetJoystickVendorForID(e->which);
				uint16_t product = SDL_GetJoystickProductForID(e->which);
				printf("%04x:%04x button %u %s\n", vendor, product, e->button, e->down ? "down" : "up");
				break;
			}
			case SDL_EVENT_JOYSTICK_HAT_MOTION:{
				SDL_JoyHatEvent *e = (SDL_JoyHatEvent *)&event;
				uint16_t vendor = SDL_GetJoystickVendorForID(e->which);
				uint16_t product = SDL_GetJoystickProductForID(e->which);
				printf("%04x:%04x hat %u %u\n", vendor, product, e->hat, e->value);
				break;
			}
			case SDL_EVENT_JOYSTICK_AXIS_MOTION:{
				SDL_JoyAxisEvent *e = (SDL_JoyAxisEvent *)&event;
				uint16_t vendor = SDL_GetJoystickVendorForID(e->which);
				uint16_t product = SDL_GetJoystickProductForID(e->which);
				printf("%04x:%04x axis %u %d\n", vendor, product, e->axis, e->value);
				break;
			}
		}
	}
}
