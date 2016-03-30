#include <_VisageTracker.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>
#include <ActionUnitBinding.h>
// VisageSDK Includes
#include <VisageTracker2.h>

using namespace VisageSDK;

VisageTracker2 *tracker;
AAssetManager *aMgr;

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jobject obj, jstring configFilename, jobject assetManager)
{
	// Init tracker
	const char *_configFilename = env->GetStringUTFChars(configFilename, 0);
	tracker = new VisageTracker2(_configFilename);

	// Init asset mananger
	aMgr = AAssetManager_fromJava(env, assetManager);

	// Initialize camera and what not
}


void Java_com_visage_visagetracker_JavaCamTracker_WriteFrameCamera(JNIEnv *env, jobject obj, jbyteArray frame)
{
	// TODO: Write code for looking at the received frame
}

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_setupBinding(JNIEnv *env, jobject obj, jstring bindFilename)
{
	// Load asset using android asset manager
	const char *_bindFilename = env->GetStringUTFChars(bindFilename, 0);
	AAsset *bindingsAsset = AAssetManager_open(aMgr, _bindFilename, AASSET_MODE_UNKNOWN);

	if(bindingsAsset == NULL)
	{
		LOGE("Could not find model bindings file");
	}

	off_t assetLength = AAsset_getLength(bindingsAsset);

	// Allocate buffer for our binding data
	char *buffer = (char*) malloc(assetLength + 1);

	if(AAsset_read(bindingsAsset, buffer, assetLength) < 0)
	{
		LOGE("Error loading bindings data in the asset manager");
		return;
	}

	// Zero-terminate
	buffer[assetLength] = 0;

	// Asset has been loaded into memory, close asset.
	AAsset_close(bindingsAsset);

	std::string textBuffer(buffer);

	// Parse buffer data into something useful
	size_t pos, linePos;
	std::string line;

	std::string delimiter = "\n";
	std::string lineDelimiter = ";";

	// Loop through the lines
	while((pos = textBuffer.find(delimiter)) != std::string::npos)
	{
		line = textBuffer.substr(0, pos);

		// Skip comments
		if(line.find('#') != std::string::npos)
		{
			textBuffer.erase(0, pos + delimiter.length());
			continue;
		}

		// NAME
		linePos = line.find(lineDelimiter);
		string name = line.substr(0, linePos);

		line.erase(0, linePos + lineDelimiter.length());

		// Blendshape Identifier
		linePos = line.find(lineDelimiter);
		string blendshapeId = line.substr(0, linePos);

		line.erase(0, linePos + lineDelimiter.length());

		// Min Limit
		linePos = line.find(lineDelimiter);
		string minLimitStr = line.substr(0, linePos);

		float minLimit = (float)atof(minLimitStr.c_str());

		// Max Limit
		linePos = line.find(lineDelimiter);
		string maxLimitStr = line.substr(0, linePos);

		float maxLimit = (float)atof(maxLimitStr.c_str());

		// Inverted
		linePos = line.find(lineDelimiter);
		string invertedStr = line.substr(0, linePos);

		bool inverted = (bool)atoi(invertedStr.c_str());

		// Weight
		linePos = line.find(lineDelimiter);
		string weightStr = line.substr(0, linePos);

		float weight = (float)atof(weightStr.c_str());

		// Filter Window
		linePos = line.find(lineDelimiter);
		string filterWindowStr = line.substr(0, linePos);

		int filterWindow = atoi(filterWindowStr.c_str());

		// Filter Amount
		linePos = line.find(lineDelimiter);
		string filterAmountStr = line.substr(0, linePos);

		float filterAmount = (float)atof(filterAmountStr.c_str());

		// Erase this line and go to next one
		textBuffer.erase(0, pos + delimiter.length());
	}


	// DO STUFF WITH THE LAST LINE
	//LOGI(textBuffer.c_str(), "LEH");

}

inline void removePreviousLineInfo(std::string &line, const size_t &linePos, const std::string &lineDelimiter)
{
	line.erase(0, linePos + lineDelimiter.length());
}
