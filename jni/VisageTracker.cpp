#include <_VisageTracker.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>
#include <sstream>
// TODO: FIX THIS CRAP, HEADER PLZ
#include <ModelLoader.cpp>
// VisageSDK Includes
#include <VisageTracker2.h>

using namespace VisageSDK;

VisageTracker2 *tracker;
ModelLoader *mLoader;

JNIEXPORT void JNICALL Java_com_visage_visagetracker_MainActivity_trackerInit(JNIEnv *env, jobject obj, jstring configFilename, jobject assetManager)
{
	const char *_configFilename = env->GetStringUTFChars(configFilename, 0);
	tracker = new VisageTracker2(_configFilename);

	// Initialize camera and what not

	// Init asset manager and model loader
	AAssetManager *aMgr = AAssetManager_fromJava(env, assetManager);
	mLoader = new ModelLoader(aMgr);
	if(mLoader->ModelExists("Jones")) {
		const aiScene *test = mLoader->LoadModel("Jones");

		if(test) {
			if(test->HasAnimations()) {
				LOGI("ANIMATIONS");
			}
			if(test->HasMaterials()) {
				LOGI("MATERIALS");
			}
			if(test->HasTextures()) {
				LOGI("TEXTURES");
			}
			if(test->HasMeshes()) {
				LOGI("MESHES");
				std::stringstream tt;
				tt << test->mNumMeshes;
				std::string tmp = tt.str();
				LOGI("%s" , tmp.c_str());

				aiMesh** meshes = test->mMeshes;
				tt << "  ";
				tt << meshes[0]->mNumVertices << "  ";
				tt << meshes[1]->mNumVertices << "  ";
				tt << meshes[2]->mNumVertices << "  ";
				tt << meshes[3]->mNumVertices << "  ";
				tt << meshes[4]->mNumVertices << "  ";
				tt << meshes[5]->mNumVertices << "  ";
				tt << meshes[6]->mNumVertices << "  ";
				tt << meshes[7]->mNumVertices << "  ";
				tmp = tt.str();
				LOGI("Anim meshes:");
				LOGI("%s" , tmp.c_str());
			}
		}
	}
}


JNIEXPORT void JNICALL Java_com_visage_visagetracker_JavaCamTracker_WriteFrameCamera(JNIEnv *env, jobject obj, jbyteArray frame)
{
	// TODO: Write code for looking at the received frame
}
