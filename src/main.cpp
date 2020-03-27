#include "ofMain.h"
#include "ofApp.h"
#include <map>

// Value-Defintions of the different String values
enum ValidArguments { debug, 
                      pal };
// Map to associate the strings with the enum values
static std::map<std::string, ValidArguments> s_mapValidArguments;

static void Initialize();
void Initialize(){
  s_mapValidArguments["--debug"] = debug;
  s_mapValidArguments["--pal"] = pal;
  cout << "s_mapStringValues contains " 
       << s_mapValidArguments.size() 
       << " entries." << endl;
}

void parseArguments(int argc, char *argv[], ofApp *app) {
    vector<string> arguments = vector<string>(argv, argv + argc);
    for (int i = 0; i < arguments.size(); ++i){
        string arg = arguments.at(i);
        switch (s_mapValidArguments[arg]) {
            case debug:
                app->debug = true;
                break;
            case pal:
                app->isPal = true;
                break;
            default:
                break;
        }
    }
}

int main(int argc, char *argv[]){
    Initialize();

    ofApp *app = new ofApp();

    parseArguments(argc, argv, app);

    ofGLESWindowSettings settings;
    if (app->isPal) {
        settings.setSize(720, 576);
    } else {
        settings.setSize(720, 480);
    }
    settings.windowMode = OF_GAME_MODE;
    settings.setGLESVersion(2);
    ofCreateWindow(settings);

    ofRunApp(app);
}
