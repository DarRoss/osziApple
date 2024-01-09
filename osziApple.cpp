/*
 * osziApple: generate an output file using Bad Apple SVG frame data.
 */

#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <math.h>
#include <iomanip>  // std::setprecision()
using namespace std;

#define NUM_FRAMES 6562
// FRAME_START minimum value: 1
#define FRAME_START 1
#define FRAME_END NUM_FRAMES
#define FPS 1024
/**
 * To be added as a keyframe, a point must be "roughly" PT_SPACING units further down the path from the last added point.
 * Setting this value to 0 maximizes image fidelity at the cost of the largest output file size.
 */
#define PT_SPACING 0
// to be added as a keyframe, a point must not be within EDGE units of the edge of the viewport
#define EDGE 30
#define SVG_PATH "svgs/"
#define SVG_WIDTH 14400
#define SVG_HEIGHT 10800
// SVG coordinates are scaled to oscilloscope coordinates using DIVISOR
#define DIVISOR ((SVG_WIDTH - 2000) / 2)
#define OUTPUT_NAME "OsziApple"
#define DECIMAL_PLACES 3
/**
 * Z_IN/Z_OUT is the Z-coordinate required for the tracer to be considered inside/outside the camera frustum.
 * Z_IN is barely inside the frustum, while Z_OUT is well outside the frustum.
 * This is to decrease the probability of trails appearing between disjoint curves.
 */
#define Z_IN 3.46
#define Z_OUT 512
/**
 * EOC_MARGIN determines the number of delay frames to add between the end of a curve and the start of another.
 * Each increment in EOC_MARGIN adds one "end of curve" and one "start of curve" delay frame.
 * A higher EOC_MARGIN value means a lower chance of trails appearing between disjoint curves, but an overall slower animation.
 */
#define EOC_MARGIN 4
// error codes
#define ERR_NONE 0
#define ERR_FCREATE_FAIL 1
#define ERR_FOPEN_FAIL 2
#define ERR_UNKNOWN_EXT 3
#define ERR_UNKNOWN_SVG_CMD 4

// list of supported output file extensions
enum outputFileExt {cs, anim};
const outputFileExt OUT_EXT = cs;

typedef struct keyframeData 
{
	double x;
	double y;
} kframe;

/**
 * printAnimPreamble: print necessary text found at the beginning of an ANIM file. 
 * fOut: the ANIM output filestream.
 */
void printAnimPreamble(ofstream &fOut)
{
	fOut << "%YAML 1.1\n"
		"%TAG !u! tag:unity3d.com,2011:\n"
		"--- !u!74 &7400000\n"
		"AnimationClip:\n"
		"  m_ObjectHideFlags: 0\n"
		"  m_CorrespondingSourceObject: {fileID: 0}\n"
		"  m_PrefabInstance: {fileID: 0}\n"
		"  m_PrefabAsset: {fileID: 0}\n"
		"  m_Name: " << OUTPUT_NAME << "\n"
		"  serializedVersion: 6\n"
		"  m_Legacy: 0\n"
		"  m_Compressed: 0\n"
		"  m_UseHighQualityCurve: 0\n"
		"  m_RotationCurves: []\n"
		"  m_CompressedRotationCurves: []\n"
		"  m_EulerCurves: []\n"
		"  m_PositionCurves:\n"
		"  - curve:\n"
		"      serializedVersion: 2\n"
		"      m_Curve:\n";
}

/**
 * printAnimFrame: given a keyframe and an index, print relevant info to an ANIM file. 
 * data: the keyframe to print.
 * index: the keyframe's index in the frame vector.
 * fOut: the ANIM output filestream.
 */
void printAnimFrame(kframe &data, int index, ofstream &fOut)
{
	fOut <<	"      - serializedVersion: 3\n"
		"        time: " << ((double)index / FPS) << "\n"
		"        value: {x: " << data.x << ", y: 0, z: " << data.y << "}\n"
		"        inSlope: {x: Infinity, y: 0, z: Infinity}\n"
		"        outSlope: {x: Infinity, y: 0, z: Infinity}\n"
		"        tangentMode: 0\n"
		"        weightedMode: 0\n"
		"        inWeight: {x: 0.33333334, y: 0.33333334, z: 0.33333334}\n"
		"        outWeight: {x: 0.33333334, y: 0.33333334, z: 0.33333334}\n";
}

/**
 * printAnimPostamble: print necessary text found at the end of an ANIM file. 
 * kframeVec: the vector of keyframes.
 * fOut: the ANIM output filestream.
 */
void printAnimPostamble(vector<kframe> &kframeVec, ofstream &fOut)
{
	int index;

	fOut << "      m_PreInfinity: 2\n"
		"      m_PostInfinity: 2\n"
		"      m_RotationOrder: 4\n"
		"    path: Armature/Bone_001\n"
		"  m_ScaleCurves: []\n"
		"  m_FloatCurves: []\n"
		"  m_PPtrCurves: []\n"
		"  m_SampleRate: " << FPS << "\n"
		"  m_WrapMode: 0\n"
		"  m_Bounds:\n"
		"    m_Center: {x: 0, y: 0, z: 0}\n"
		"    m_Extent: {x: 0, y: 0, z: 0}\n"
		"  m_ClipBindingConstant:\n"
		"    genericBindings:\n"
		"    - serializedVersion: 2\n"
		"      path: 2729491044\n"
		"      attribute: 1\n"
		"      script: {fileID: 0}\n"
		"      typeID: 4\n"
		"      customType: 0\n"
		"      isPPtrCurve: 0\n"
		"    pptrCurveMapping: []\n"
		"  m_AnimationClipSettings:\n"
		"    serializedVersion: 2\n"
		"    m_AdditiveReferencePoseClip: {fileID: 0}\n"
		"    m_AdditiveReferencePoseTime: 0\n"
		"    m_StartTime: 0\n"
		"    m_StopTime: " << ((double)kframeVec.size() / FPS) << "\n"
		"    m_OrientationOffsetY: 0\n"
		"    m_Level: 0\n"
		"    m_CycleOffset: 0\n"
		"    m_HasAdditiveReferencePose: 0\n"
		"    m_LoopTime: 1\n"
		"    m_LoopBlend: 0\n"
		"    m_LoopBlendOrientation: 0\n"
		"    m_LoopBlendPositionY: 0\n"
		"    m_LoopBlendPositionXZ: 0\n"
		"    m_KeepOriginalOrientation: 0\n"
		"    m_KeepOriginalPositionY: 1\n"
		"    m_KeepOriginalPositionXZ: 0\n"
		"    m_HeightFromFeet: 0\n"
		"    m_Mirror: 0\n"
		"  m_EditorCurves:\n"
		"  - curve:\n"
		"      serializedVersion: 2\n"
		"      m_Curve:\n";

	// x coordinate keyframe data
	for(index = 0; index < (int)kframeVec.size(); ++index)
	{
		fOut << "      - serializedVersion: 3\n"
			"        time: " << ((double) index / FPS) << "\n"
			"        value: " << kframeVec.at(index).x << "\n"
			"        inSlope: Infinity\n"
			"        outSlope: Infinity\n"
			"        tangentMode: 103\n"
			"        weightedMode: 0\n"
			"        inWeight: 0.33333334\n"
			"        outWeight: 0.33333334\n";
	}

	fOut << "      m_PreInfinity: 2\n"
		"      m_PostInfinity: 2\n"
		"      m_RotationOrder: 4\n"
		"    attribute: m_LocalPosition.x\n"
		"    path: Armature/Bone_001\n"
		"    classID: 4\n"
		"    script: {fileID: 0}\n"
		"  - curve:\n"
		"      serializedVersion: 2\n"
		"      m_Curve:\n";

	// z coordinate keyframe data
	for(index = 0; index < (int)kframeVec.size(); ++index)
	{
		fOut << "      - serializedVersion: 3\n"
			"        time: " << ((double) index / FPS) << "\n"
			"        value: " << kframeVec.at(index).y << "\n"
			"        inSlope: Infinity\n"
			"        outSlope: Infinity\n"
			"        tangentMode: 103\n"
			"        weightedMode: 0\n"
			"        inWeight: 0.33333334\n"
			"        outWeight: 0.33333334\n";
	}

	fOut << "      m_PreInfinity: 2\n"
		"      m_PostInfinity: 2\n"
		"      m_RotationOrder: 4\n"
		"    attribute: m_LocalPosition.z\n"
		"    path: Armature/Bone_001\n"
		"    classID: 4\n"
		"    script: {fileID: 0}\n"
		"  m_EulerEditorCurves: []\n"
		"  m_HasGenericRootTransform: 0\n"
		"  m_HasMotionFloatCurves: 0\n"
		"  m_Events: []";
}

/**
 * printMelonFile: print the entire CSharp MelonLoader file using vector keyframe data. 
 * kframeVec: the vector of keyframes.
 * eocVec: the vector of end of curve frame numbers.
 * fOut: the CS output filestream.
 */
void printMelonFile(vector<kframe> &kframeVec, vector<int> &eocVec, ofstream &fOut)
{
	int index;

	// set the number of decimal places to be used
	fOut << fixed << setprecision(DECIMAL_PLACES);

	fOut << "using MelonLoader;\n"
		"using UnityEngine;\n"
		"using UnityEngine.SceneManagement;\n"
		"namespace OsziAppleMod\n"
		"{\n"
		"\tpublic class Storage\n"
		"\t{\n"
		"\t\tpublic bool isLoaded = false;\n"
		"\t}\n"
		"\tpublic class OsziApple : MelonMod\n"
		"\t{\n"
		"\t\tStorage storage;\n"
		"\t\tpublic override void OnUpdate()\n"
		"\t\t{\n"
		"\t\t\tif(storage == null)\n"
		"\t\t\t{\n"
		"\t\t\t\tstorage = new Storage();\n"
		"\t\t\t}\n"
		"\t\t\tif(!storage.isLoaded && SceneManager.GetActiveScene().name == \"LAB_Labyrinth\")\n"
		"\t\t\t{\n";
	// xy-coordinate data array
	// each even index holds an X-coordinate and each odd index holds a Y-coordinate
	fOut << "\t\t\t\tdouble[] dataXY = {";
	fOut << kframeVec.at(0).x << "," << kframeVec.at(0).y;
	for(index = 1; index < (int)kframeVec.size(); ++index)
	{
		fOut << "," << kframeVec.at(index).x << "," << kframeVec.at(index).y;
	}
	fOut << "};\n";
	// end of curve frame number array
	// holds the set of keyframe numbers at which the tracer is outside the camera frustum and is invisible
	fOut << "\t\t\t\tint[] eocIndices = {";
	fOut << eocVec.at(0);
	for(index = 1; index < (int)eocVec.size(); ++index)
	{
		fOut << "," << eocVec.at(index);
	}
	fOut << "};\n";
	// constants
	fOut << "\t\t\t\tstring clipName = \"" << OUTPUT_NAME << "\";\n"
		"\t\t\t\tint fps = " << FPS << ";\n"
		"\t\t\t\tint vecLen = " << kframeVec.size() << ";\n"
		"\t\t\t\tint eocLen = " << eocVec.size() << ";\n"
		"\t\t\t\tint eocMargin = " << EOC_MARGIN << ";\n"
		"\t\t\t\tfloat zIn = " << Z_IN << "f;\n"
		"\t\t\t\tfloat zOut = " << Z_OUT << "f;\n"
		"\t\t\t\tstring[] dims = {\"x\", \"y\"};\n";
	// other variables
	fOut << "\t\t\t\tGameObject osziObj = GameObject.Find(\"/Events/LAB_PatternPond/Oszilloskop\");\n"
		"\t\t\t\tAnimation animn = osziObj.GetComponent<Animation>();\n"
		"\t\t\t\tAnimationClip clip = new AnimationClip();\n"
		"\t\t\t\tKeyframe[] keys = new Keyframe[vecLen];\n"
		"\t\t\t\tKeyframe[] keysEoc = new Keyframe[eocLen * 3];\n"
		"\t\t\t\tint index;\n"
		"\t\t\t\tint dimInd;\n";
	// beginning of setup
	fOut << "\t\t\t\tif (!animn) animn = osziObj.AddComponent<Animation>();\n"
		"\t\t\t\tclip.name = clipName;\n"
		"\t\t\t\tclip.legacy = true;\n";
	// xy-coordinate keyframe generation
	fOut << "\t\t\t\tfor(dimInd = 0; dimInd < dims.Length; ++dimInd)\n"
		"\t\t\t\t{\n"
		"\t\t\t\t\tfor(index = 0; index < vecLen; ++index)\n"
		"\t\t\t\t\t{\n"
		"\t\t\t\t\t\tkeys[index] = new Keyframe((float)index / fps, (float)dataXY[index * dims.Length + dimInd]);\n"
		"\t\t\t\t\t}\n"
		"\t\t\t\t\tclip.SetCurve(\"Armature/Bone_001\", Transform.Il2CppType, \"localPosition.\" + dims[dimInd], new AnimationCurve(keys));\n"
		"\t\t\t\t}\n";
	// z-coordinate keyframe generation
	fOut << "\t\t\t\tfor(index = 0; index < eocLen; ++index)\n"
		"\t\t\t\t{\n"
		"\t\t\t\t\tkeysEoc[index * 3] = new Keyframe((float)(eocIndices[index] - eocMargin) / fps, zIn);\n"
		"\t\t\t\t\tkeysEoc[index * 3 + 1] = new Keyframe((float)(eocIndices[index]) / fps, zOut);\n"
		"\t\t\t\t\tkeysEoc[index * 3 + 2] = new Keyframe((float)(eocIndices[index] + eocMargin) / fps, zIn);\n"
		"\t\t\t\t}\n"
		"\t\t\t\tclip.SetCurve(\"Armature/Bone_001\", Transform.Il2CppType, \"localPosition.z\", new AnimationCurve(keysEoc));\n";
	// additional setup
	fOut << "\t\t\t\tosziObj.GetComponent<Animator>().enabled = false;\n"
		"\t\t\t\tanimn.clip = clip;\n"
		"\t\t\t\tanimn.AddClip(clip, clip.name);\n"
		"\t\t\t\tanimn.wrapMode = WrapMode.Loop;\n"
		"\t\t\t\tanimn.Play();\n"
		"\t\t\t\tstorage.isLoaded = true;\n"
		"\t\t\t\tMelonLogger.Msg(\"Bad Apple Loaded Successfully\");\n"
		"\t\t\t}\n"
		"\t\t}\n"
		"\t}\n"
		"}";
}

/**
 * processLastCmdWord: remove any trailing characters at the end of the last word in an SVG path command. 
 * This then allows the word to be used with stoi().
 * word: the last word in an SVG path command.
 * return true if this is the end of the path.
 */
bool processLastCmdWord(string &word)
{
	bool endOfPath = false;
	if(word.back() == '>')
	{
		// remove the last 4 chars (z"/>) from the word
		word.erase(word.length() - 4);
		// this marks the end of the path (and as such, the end of this frame's SVG file)
		endOfPath = true;
	}
	else if(word.back() == 'z')
	{
		// remove the last char (z)
		word.pop_back();
	}

	return endOfPath;
}

/**
 * performCmd: given a SVG command, perform necessary translations. 
 * currCmd: the provided SVG command (M, m, c, l). 
 * frameOld: the keyframe data of the previous frame. 
 * frameNew: the keyframe data of the current frame. 
 * fIn: the input file stream. 
 * endOfPath: true if and only if this curve has been traversed in its entirety.
 * distSum: a rough estimate for the current path length.
 * firstWord: the X-coordinate that was literally attached to the given command. 
 * return 0 if and only if no errors occurred.
 */
int performCmd(char currCmd, kframe &frameOld, kframe &frameNew, ifstream &fIn, bool &endOfPath, double &distSum, string firstWord)
{
	int retVal = 0;
	string strIn = firstWord;

	switch(currCmd)
	{
		// move to
		case 'M':
			frameNew.x = stoi(strIn);
			fIn >> strIn;
			frameNew.y = stoi(strIn);
			// distance sum is reset upon moving to a new curve
			distSum = 0;
			frameOld = frameNew;
			break;
		// move to (relative)
		case 'm':
			frameNew.x = frameOld.x + stoi(strIn);
			fIn >> strIn;
			frameNew.y = frameOld.y + stoi(strIn);
			// distance sum is reset upon moving to a new curve
			distSum = 0;
			frameOld = frameNew;
			break;
		// bezier curve (relative)
		case 'c': 
			// discard curvature data because curve length is "approximated" as a line (lazy implementation)
			fIn >> strIn >> strIn >> strIn >> strIn;
			[[fallthrough]];
		// line (relative)
		case 'l':
			frameNew.x = frameOld.x + stoi(strIn);
			fIn >> strIn;
			endOfPath = processLastCmdWord(strIn);
			frameNew.y = frameOld.y + stoi(strIn);
			break;
		default:
			retVal = ERR_UNKNOWN_SVG_CMD;
			cout << "Could not recognize SVG path command \'" << currCmd << "\'." << endl;
	}
	return retVal;
}

/**
 * processKeyframe: given a new keyframe, process and push it to the keyframe vector.
 * kframeVec: the vector of keyframes. 
 * eocVec: the vector of end of curve frame numbers.
 * frameOld: the keyframe data of the previous frame. 
 * frameNew: the keyframe data of the current frame. 
 * fOut: the output file stream. 
 * distSum: a rough estimate for the current path length.
 * newCurve: true if and only if we have moved to a new, disjoint curve. 
 */
void processKeyframe(vector<kframe> &kframeVec, vector<int> &eocVec, kframe &frameOld, kframe &frameNew, ofstream &fOut, double &distSum, bool &newCurve)
{
	// frame data in terms of oscilloscope coordinates
	kframe frameOszi;
	kframe temp;
	int index;

	// do not add a point on or near the edge
	if(frameNew.x >= EDGE && frameNew.y >= EDGE && frameNew.x <= SVG_WIDTH - EDGE && frameNew.y <= SVG_HEIGHT - EDGE)
	{
		distSum += hypot(frameNew.x - frameOld.x, frameNew.y - frameOld.y);
		// only add a point if it is far enough away from the previous point
		if(distSum >= PT_SPACING)
		{
			distSum = 0;
			// convert to oscilloscope coordinates
			frameOszi.x = -(double)(frameNew.x - SVG_WIDTH / 2) / DIVISOR;
			frameOszi.y = (double)(frameNew.y - SVG_HEIGHT / 2) / DIVISOR;
			switch(OUT_EXT)
			{
				case anim:
					printAnimFrame(frameOszi, kframeVec.size(), fOut);
					break;
				default:
					// do nothing
					break;
			}
			if(newCurve && kframeVec.size() > 0)
			{
				newCurve = false;
				// the latest keyframe serves as an "end of curve" delay frame
				temp = kframeVec.back();
				// add some "end of curve" delay frames based on the end of curve margin size
				for(index = 0; index < EOC_MARGIN; ++index)
				{
					kframeVec.push_back(temp);
				}
				// the index of the first "start of curve" delay frame is marked as end of curve
				eocVec.push_back(kframeVec.size());
				// add some "start of curve" delay frames based on the end of curve margin size
				for(index = 0; index < EOC_MARGIN; ++index)
				{
					kframeVec.push_back(frameOszi);
				}
			}
			kframeVec.push_back(frameOszi);
		}
	}
	else
	{
		// an omitted point near the edge will act as a break in the curve
		newCurve = true;
		distSum = 0;
	}
	frameOld = frameNew;
}

/**
 * processSvgFile: Given an SVG file, process its point data and write keyframe data to the output file.
 * kframeVec: the vector of keyframe data. 
 * eocVec: the vector of end of curve frame numbers.
 * fIn: the SVG input filestream. 
 * fOut: the output filestream. 
 * return 0 if and only if no errors occurred.
 */
int processSvgFile(vector<kframe> &kframeVec, vector<int> &eocVec, ifstream &fIn, ofstream &fOut)
{
	kframe frameOld;
	kframe frameNew;
	string strIn;
	bool endOfPath;
	bool newCurve = false;
	// retVal == 0 when there are no errors
	int retVal = 0;
	char currCmd;
	// the distance sum of traversed curves. useful in conjuction with PT_SPACING
	double distSum = 0;

	while(fIn >> strIn && !retVal)
	{
		// check if this is the start of a path
		if(strIn == "<path")
		{
			endOfPath = false;
			fIn >> strIn;
			// remove first 3 chars (d=") from the first word in the path
			strIn.erase(0, 3);
			do
			{
				// change currCmd if the input word starts with a non-number
				if(strIn.at(0) != '-' && (strIn.at(0) < '0' || strIn.at(0) > '9'))
				{
					currCmd = strIn.at(0);
					// remove the command character so that we may process the 
					// x-coordinate attached to it
					strIn.erase(0, 1);
					// check if this is a move command, implying a new curve
					if(currCmd == 'M' || currCmd == 'm')
					{
						newCurve = true;
					}
				}
				retVal = performCmd(currCmd, frameOld, frameNew, fIn, endOfPath, distSum, strIn);
				if(!retVal)
				{
					processKeyframe(kframeVec, eocVec, frameOld, frameNew, fOut, distSum, newCurve);
				}
			}
			while(fIn >> strIn && !endOfPath && !retVal);
		}
	}
	return retVal;
}

/**
 * getExtension: obtain the desired output file extension.
 * extension: the string to store this file extension.
 * return 0 if and only if no errors occur.
 */
int getExtension(string &extension)
{
	int retVal = 0;
	switch(OUT_EXT)
	{
		case cs:
			extension = "cs";
			break;
		case anim:
			extension = "anim";
			break;
		default:
			retVal = ERR_UNKNOWN_EXT;
			cout << "Could not recognize extension " << OUT_EXT << endl;
	}
	return retVal;
}

/**
 * printPreamble: print to the output file preliminary stuff that is known before processing SVGs. 
 * fOut: the output file stream.
 */
void printPreamble(ofstream &fOut)
{
	switch(OUT_EXT)
	{
		case anim:
			printAnimPreamble(fOut);
			break;
		default:
			// do nothing
			break;
	}
}

/**
 * printPostamble: print to the output file stuff that is only available after processing SVGs. 
 * kframeVec: the vector of keyframes. 
 * eocVec: the vector of end of curve frame numbers.
 * fOut: the output file stream.
 */
void printPostamble(vector<kframe> &kframeVec, vector<int> &eocVec, ofstream &fOut)
{
	switch(OUT_EXT)
	{
		case anim:
			printAnimPostamble(kframeVec, fOut);
			break;
		case cs:
			printMelonFile(kframeVec, eocVec, fOut);
			break;
		default:
			// do nothing
			break;
	}
}

/**
 * traverseSvgFiles: loop through desired SVG files in the SVG path and process each of them. 
 * kframeVec: the vector of keyframes. 
 * eocVec: the vector of end of curve frame numbers.
 * fOut: the output file stream.
 * return 0 if and only if no errors occur.
 */
int traverseSvgFiles(vector<kframe> &kframeVec, vector<int> &eocVec, ofstream &fOut)
{
	int retVal = 0;
	// SVG input filestream
	ifstream fIn;
	int index;
	string fileName;

	for(index = FRAME_START; index <= FRAME_END && !retVal; ++index)
	{
		fileName = to_string(index);
		// pad filename with zeros
		fileName = SVG_PATH + string(4 - min(4, (int)fileName.length()), '0') + fileName + ".svg";
		fIn.open(fileName, ios::in);
		if(fIn)
		{
			retVal = processSvgFile(kframeVec, eocVec, fIn, fOut);
			fIn.close();
			if(retVal)
			{
				cout << "Error occurred at file " << fileName << endl;
			}
		}
		else
		{
			retVal = ERR_FOPEN_FAIL;
			cout << "Could not open file " << fileName << endl;
		}
	}
	return retVal;
}

/**
 * printRetMsg: print either an error or success message based on the return value.
 * retVal: the given return value.
 */
void printRetMsg(int retVal)
{
	switch(retVal)
	{
		case ERR_NONE:
			cout << "Output file successfully generated" << endl;
			break;
		case ERR_FCREATE_FAIL:
			cout << "Error: failure to create output file" << endl;
			break;
		case ERR_FOPEN_FAIL:
			cout << "Error: failure to open SVG file" << endl;
			break;
		case ERR_UNKNOWN_EXT:
			cout << "Error: unknown output file extension" << endl;
			break;
		case ERR_UNKNOWN_SVG_CMD:
			cout << "Error: unknown SVG command" << endl;
			break;
		default:
			cout << "Unknown error encountered" << endl;
			break;
	}
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	
	vector<kframe> kframeVec;
	// the end of curve vector contains frame numbers that mark the end of a continuous curve before moving to the next
	vector<int> eocVec;
	// output filestream
	ofstream fOut;
	int retVal = 0;
	// the output file extension
	string extension;

	retVal = getExtension(extension);
	if(!retVal)
	{
		fOut.open(OUTPUT_NAME "." + extension, ios::out);
		if(fOut)
		{
			printPreamble(fOut);
			// traverse SVG files containing frame data
			retVal = traverseSvgFiles(kframeVec, eocVec, fOut);
			if(!retVal)
			{
				// print postamble if all input files were successful
				printPostamble(kframeVec, eocVec, fOut);
			}
			fOut.close();
		}
		else
		{
			retVal = ERR_FCREATE_FAIL;
		}
	}
	printRetMsg(retVal);
	return retVal;
}
