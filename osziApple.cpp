/*
 * osziApple: generate an output file using Bad Apple SVG frame data.
 */

#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <math.h>
using namespace std;

#define NUM_FRAMES 6562
#define FPS 60
// to be added as a keyframe, a point must be roughly this far down the path from the last added point
#define PT_SPACING 512
// to be added as a keyframe, a point must not be this close to the edge of the viewport
#define EDGE 20
#define SVG_PATH "svgs/"
#define SVG_WIDTH 14400
#define SVG_HEIGHT 10800
// SVG coordinates are scaled to normalized coordinates using DIVISOR
#define DIVISOR (SVG_WIDTH / 2)
#define OUTPUT_NAME "Oszipattern_Bad_Apple"
#define ERR_CODE 1

// list of supported output file extensions
enum outputFileExt {cs, anim};
const outputFileExt OUT_EXT = anim;

typedef struct keyframeData 
{
	double x;
	double y;
} frame;

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
void printAnimFrame(frame &data, int index, ofstream &fOut)
{
	fOut <<	"      - serializedVersion: 3\n"
		"        time: " << to_string((double)index / FPS) << "\n"
		"        value: {x: " << to_string(data.x) << ", y: 0, z: " << to_string(data.y) << "}\n"
		"        inSlope: {x: Infinity, y: 0, z: Infinity}\n"
		"        outSlope: {x: Infinity, y: 0, z: Infinity}\n"
		"        tangentMode: 0\n"
		"        weightedMode: 0\n"
		"        inWeight: {x: 0.33333334, y: 0.33333334, z: 0.33333334}\n"
		"        outWeight: {x: 0.33333334, y: 0.33333334, z: 0.33333334}\n";
}

/**
 * printAnimPostamble: print necessary text found at the end of an ANIM file. 
 * frameVec: the vector of keyframes.
 * fOut: the ANIM output filestream.
 */
void printAnimPostamble(vector<frame> &frameVec, ofstream &fOut)
{
	int index;
	fOut << "      m_PreInfinity: 2\n"
		"      m_PostInfinity: 2\n"
		"      m_RotationOrder: 4\n"
		"    path: Armature/Bone_001\n"
		"  m_ScaleCurves: []\n"
		"  m_FloatCurves: []\n"
		"  m_PPtrCurves: []\n"
		"  m_SampleRate: " << FPS<< "\n"
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
		"    m_StopTime: " << to_string((double)frameVec.size() / FPS) << "\n"
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
	for(index = 0; index < (int)frameVec.size(); ++index)
	{
		fOut << "      - serializedVersion: 3\n"
			"        time: " << to_string((double) index / FPS) << "\n"
			"        value: " << to_string(frameVec.at(index).x) << "\n"
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
	for(index = 0; index < (int)frameVec.size(); ++index)
	{
		fOut << "      - serializedVersion: 3\n"
			"        time: " << to_string((double) index / FPS) << "\n"
			"        value: " << to_string(frameVec.at(index).y) << "\n"
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
		"  m_Events: []\n";
}

/**
 * printCsFile: print the entire CSharp script file using vector keyframes. 
 * frameVec: the vector of keyframes.
 * fOut: the CS output filestream.
 */
void printCsFile(vector<frame> &frameVec, ofstream &fOut)
{
	(void) frameVec;
	(void) fOut;

	// TODO
}

/**
 * processLastCmdWord: remove any trailing characters at the end of the last word in an SVG path command. 
 * This results in word compatibility with stoi().
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
		// this marks the end of the path
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
 * pushSvgKeyframes: Given an SVG file, push new keyframe data onto a vector of frames. 
 * Depending on output extension, write keyframe data to output file.
 * frameVec: the vector of keyframe data. 
 * fIn: the SVG input filestream. 
 * fOut: the ANIM output filestream. 
 * return 0 if no errors occurred.
 */
int pushSvgKeyframes(vector<frame> &frameVec, ifstream &fIn, ofstream &fOut)
{
	(void)frameVec;
	(void)fOut;

	frame frameOld;
	frame frameNew;
	frame frameNormd;
	string strIn;
	bool endOfPath;
	// retVal == 0 when there are no errors
	int retVal = 0;
	char currCmd;
	// the distance sum of traversed lines and curves
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
				// change currCmd if the input word starts with a letter
				if(strIn.at(0) != '-' && (strIn.at(0) < '0' || strIn.at(0) > '9'))
				{
					currCmd = strIn.at(0);
					strIn.erase(0, 1);
				}

				switch(currCmd)
				{
					// move to
					case 'M':
						frameNew.x = stoi(strIn);
						fIn >> strIn;
						frameNew.y = stoi(strIn);
						// distance sum is reset upon moving
						distSum = 0;
						frameOld = frameNew;
						break;
					// move to (relative)
					case 'm':
						frameNew.x = frameOld.x + stoi(strIn);
						fIn >> strIn;
						frameNew.y = frameOld.y + stoi(strIn);
						// distance sum is reset upon moving
						distSum = 0;
						frameOld = frameNew;
						break;
					// bezier curve (relative)
					case 'c': 
						// discard curvature data because curve length is "approximated" as a line
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
						retVal = ERR_CODE;
						cout << "Error: unknown SVG path command \'" << currCmd << "\'." << endl;
				}

				if(!retVal)
				{
					// do not add a point on or near the edge
					if(frameNew.x > EDGE && frameNew.y > EDGE && frameNew.x < SVG_WIDTH - EDGE && frameNew.y < SVG_HEIGHT - EDGE)
					{
						distSum += hypot(frameNew.x - frameOld.x, frameNew.y - frameOld.y);
						// only add a point if it is far enough away from the previous point
						if(distSum >= PT_SPACING)
						{
							distSum = 0;
							frameNormd.x = -(double)(frameNew.x - SVG_WIDTH / 2) / DIVISOR;
							frameNormd.y = -(double)(frameNew.y - SVG_HEIGHT / 2) / DIVISOR;
							switch(OUT_EXT)
							{
								case anim:
									printAnimFrame(frameNormd, frameVec.size(), fOut);
									break;
								default:
									// do nothing
									break;
							}
							frameVec.push_back(frameNormd);
						}
					}
					else
					{
						distSum = 0;
					}
					frameOld = frameNew;
				}
			}
			while(fIn >> strIn && !endOfPath && !retVal);
		}
	}

	return retVal;
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	
	vector<frame> frameVec;
	// output filestream
	ofstream fOut;
	// SVG input filestream
	ifstream fIn;

	// the input file for-loop will check if retVal is 0 (no errors) before continuing
	int retVal = 0;
	int index;
	string fileName;
	string extension;

	switch(OUT_EXT)
	{
		case cs:
			extension = "cs";
			break;
		case anim:
			extension = "anim";
			break;
		default:
			retVal = ERR_CODE;
			cout << "Error: unknown extension with value " << OUT_EXT << endl;
	}

	if(!retVal)
	{
		fOut.open(OUTPUT_NAME "." + extension, ios::out);
		if(fOut)
		{
			// print preamble depending on output file extension
			switch(OUT_EXT)
			{
				case anim:
					printAnimPreamble(fOut);
					break;
				default:
					// do nothing
					break;
			}

			// traverse SVG files containing frame data
			for(index = 1; index <= NUM_FRAMES && !retVal; ++index)
			{
				fileName = to_string(index);
				// pad filename with zeros
				fileName = SVG_PATH + string(4 - min(4, (int)fileName.length()), '0') + fileName + ".svg";

				fIn.open(fileName, ios::in);
				if(fIn)
				{
					retVal = pushSvgKeyframes(frameVec, fIn, fOut);
					fIn.close();
					if(retVal)
					{
						cout << "Error at file " << fileName << endl;
					}
				}
				else
				{
					retVal = ERR_CODE;
					cout << "Error: unable to open file " << fileName << endl;
				}
			}

			// print postamble if all input files were successful
			if(!retVal)
			{
				switch(OUT_EXT)
				{
					case anim:
						printAnimPostamble(frameVec, fOut);
						break;
					case cs:
						printCsFile(frameVec, fOut);
						break;
					default:
						// do nothing
						break;
				}

				cout << "Output file (" << OUTPUT_NAME << "." << extension << ") successfully generated." << endl;
			}
			fOut.close();
		}
		else
		{
			retVal = ERR_CODE;
			cout << "Error: unable to create file " << OUTPUT_NAME << "." << extension << endl;
		}
	}

	return retVal;
}
