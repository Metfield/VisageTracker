#include <_VisageTracker.h>

// VisageSDK Includes
#include <VisageTracker2.h>

using namespace VisageSDK;

VisageTracker2 *tracker;

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jobject obj, jstring configFilename)
{
	const char *_configFilename = env->GetStringUTFChars(configFilename, 0);
	tracker = new VisageTracker2(_configFilename);

	// Initialize camera and what not
}


void Java_com_visage_visagetracker_JavaCamTracker_WriteFrameCamera(JNIEnv *env, jobject obj, jbyteArray frame)
{
	// TODO: Write code for looking at the received frame
}

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_setupBinding(JNIEnv *env, jobject obj, jstring bindFilename)
{
	std::string textBuffer = "au_jaw_x_push;					skin:15;					0;			0.5;		0;			1.0;		4;					0.3; \n\
au_jaw_y_push;					teeth:0;					0;			0.5;		0;			1.0;		4;					0.3; \n\
au_jaw_z_push;					tongue:0;					0;			0.5;		0;			1.0;		4;					0.3;";
	std::string delimiter = "\n";

	size_t pos, linePos;
	std::string line;

	while((pos = textBuffer.find(delimiter)) != std::string::npos)
	{
		line = textBuffer.substr(0, pos);

		// DO STUFF WITH THE LINE
		// Skip comments
		/*if(line.find("#"))
		{
			textBuffer.erase(0, pos + delimiter.length());
			continue;
		}*/


		linePos = line.find(";");
		string name = line.substr(0, linePos);

		LOGI(name.c_str(), "YAY");


		LOGI(line.c_str(), "kiss mah ass!");

		textBuffer.erase(0, pos + delimiter.length());
	}


	// DO STUFF WITH THE LAST LINE
	LOGI(textBuffer.c_str(), "LEH");


	/*

	foreach (string line in lines)
	{
		// skip comments
		if (line.StartsWith("#"))
			continue;

		string[] values = line.Split(new [] { ";" }, StringSplitOptions.RemoveEmptyEntries);
		string[] trimmedValues = new string[8];
		for (int i = 0; i < Mathf.Min(values.Length, 8); i++)
		{
			// trim values
			trimmedValues[i] = values[i].Trim();
		}

		// parse au name
		string au = trimmedValues[0];

		// parse blendshape identifier
		string blendshape = trimmedValues[1];
		string[] blendShapeParts = blendshape.Split(':');
		if (blendShapeParts.Length < 2)
		{
			Debug.LogError("Invalid blendshape_indentifier value in configuration '" + configuration.name + "'.", tracker);
			return;
		}

		string blendshapeObjectName = blendShapeParts[0];
		int blendshapeIndex = 0;
		if (!int.TryParse(blendShapeParts[1], out blendshapeIndex))
		{
			Debug.LogError("Invalid blendshape_indentifier value in configuration '" + configuration.name + "'.", tracker);
			return;
		}

		GameObject target = GameObject.Find(blendshapeObjectName);
		if (target == null || target.GetComponent<SkinnedMeshRenderer>() == null)
		{
			Debug.LogError("No valid blendshape target named '" + blendshapeObjectName + "' defined in configuration: '" + configuration.name + "'.", tracker);
			return;
		}

		// parse min limit
		float min = -1f;
		if (!float.TryParse(trimmedValues[2], System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out min))
		{
			Debug.LogError("Invalid min_limit value in binding configuration '" + configuration.name + "'.", tracker);
			return;
		}

		// parse max limit
		float max = 1f;
		if (!float.TryParse(trimmedValues[3], System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out max))
		{
			Debug.LogError("Invalid max_limit value in binding configuration '" + configuration.name + "'.", tracker);
			return;
		}

		// parse inverted
		bool inverted = false;
		if (!string.IsNullOrEmpty(trimmedValues[4]))
			inverted = trimmedValues[4] == "1";

		// parse weight
		float weight = 1f;
		if (!string.IsNullOrEmpty(trimmedValues[5]))
		{
			if (!float.TryParse(trimmedValues[5], System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out weight))
			{
				Debug.LogError("Invalid weight value in binding configuration '" + configuration.name + "'.", tracker);
				return;
			}
		}

		// parse filter window
		int filterWindow = 6;
		if (!string.IsNullOrEmpty(trimmedValues[6]))
		{
			if (!int.TryParse(trimmedValues[6], out filterWindow) || filterWindow < 0 || filterWindow > 16)
			{
				Debug.LogError("Invalid filter_window value in binding configuration '" + configuration.name + "'.", tracker);
				return;
			}
		}

		// parse filter amount
		float filterAmount = 0.3f;
		if (!string.IsNullOrEmpty(trimmedValues[7]))
		{
			if (!float.TryParse(trimmedValues[7], System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out filterAmount))
			{
				Debug.LogError("Invalid filter_amount value in binding configuration '" + configuration.name + "'.", tracker);
				return;
			}
		}

		// add new binding
		ActionUnitBinding binding = tracker.gameObject.AddComponent<ActionUnitBinding>();
		binding.Name = au + " -> " + blendshape;
		binding.Tracker = tracker;
		binding.ActionUnitName = au;
		binding.Limits = new Vector2(min, max);
		binding.Inverted = inverted;
		binding.Weight = weight;
		binding.FilterWindowSize = filterWindow;
		binding.FilterConstant = filterAmount;
		binding.Targets = new ActionUnitBindingTarget[1];
		binding.Targets[0] = new ActionUnitBindingTarget();
		binding.Targets[0].Renderer = GameObject.Find(blendshapeObjectName).GetComponent<SkinnedMeshRenderer>();
		binding.Targets[0].BlendshapeIndex = blendshapeIndex;
		binding.Targets[0].Weight = 1f;
	}*/
}
