#include "KeystrokeToWordConverter.h"

#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

bool KeystrokeToWordConverter::loadModel(string& lmFilePath)
{
	if(lm_ = new LanguageModel(lmFilePath))
		return true;
	else
		return false;
}

bool KeystrokeToWordConverter::loadDictionary(string& dicFilePath)
{
	if(dic_ = new Dictionary(dicFilePath))
		return true;
	else
		return false;
}

bool KeystrokeToWordConverter::convert(vector<Token*>& inputTokens, vector<Token*>& outputTokens)
{
	size_t length = inputTokens.size();
	cerr << "input tokens: " << length << endl;
	for(size_t shift = 0; shift < length; shift++)
		cerr << inputTokens[shift]->characterStringVector[0] << endl;		
	cerr << "----" << endl;
	vector<double> scores(length + 1);
	vector<int> tracks(length + 1);
	vector<string> words(length + 1);
	for(size_t i = 0; i < length + 1; i++)
	{
		scores[i] = 0.0;
		tracks[i] = -1;
		words[i] = "";
	}

	for(size_t index = 1; index <= length; index++)
	{
		double bestScore = 0.0f;
		int bestPrefix = -1;
		string bestWord = "";
		for(int prefix = index - 1; prefix >= 0; prefix--)
		{
			cerr << endl << "(" << prefix << "," << index << "):";
			string rightKey("");
			for(size_t shift = prefix; shift < index; shift++)
				rightKey += inputTokens[shift]->characterStringVector[0] + " ";
			rightKey = rightKey.substr(0, rightKey.length() - 1);
			cerr << "{" << rightKey << "}";
			vector<string> rightGrams;
			size_t rightGramCount = dic_->getValueVector(rightKey, rightGrams);
			cerr << "[" << rightGramCount << "]" << endl;
			for(size_t i = 0; i < rightGramCount; i++)
			{
				string rightGram = rightGrams[i];
				cerr << i << "." << rightGram.c_str() << ":";
			
				if(lm_->has(rightGram)) {
					int left = tracks[prefix];
					if(left >= 0 && left != prefix)
					{
						string leftKey("");
						for(size_t shift = left; shift < prefix; shift++)
							leftKey += inputTokens[shift]->characterStringVector[0] + " ";
						leftKey = leftKey.substr(0, leftKey.length() - 1);
						vector<string> leftGrams;
						size_t leftGramCount =
							dic_->getValueVector(leftKey, leftGrams);
						for(size_t j = 0; j < leftGramCount; j++)
						{
							double tempScore = 0.0f;
							string leftGram = leftGrams[j];
							string bigram = leftGram + " " + rightGram;
							cerr << "(test bigram:" << bigram << ")";
							if(lm_->has(bigram)) {
								double bigramScore = lm_->getLogProb(bigram);
								cerr << bigramScore << "(bigram) + ";
								tempScore += bigramScore;
							}
							else if(dic_->has(leftGram)) {
								double bigramBackOff =
									lm_->getLogProb(leftGram) +
									lm_->getBackOff(rightGram);
								cerr << bigramBackOff << "(backoff) + ";
								tempScore += bigramBackOff;
							}
							else {
								tempScore +=
									lm_->getUnkLogProb() +
									lm_->getBackOff(rightGram);
								cerr << (lm_->getUnkLogProb() + lm_->getBackOff(rightGram));
								cerr << "(unknown) + ";
							}
							
							tempScore += scores[prefix];
							cerr << scores[prefix] << "(scores[prefix])" << endl;
							if(bestScore == 0.0f || tempScore > bestScore)
							{
								bestScore = tempScore;
								bestPrefix = prefix;
								bestWord = rightGram;
								cerr << "argmax=" << prefix;
								cerr << "," << bestScore;
								cerr <<" ," << bestWord << endl;
							}
						}
					}
					else {
						double tempScore = lm_->getLogProb(rightGram);
						cerr << tempScore << "(init)" << endl;
						if(bestScore == 0.0f || tempScore > bestScore)
						{
							bestScore = tempScore;
							bestPrefix = prefix;
							bestWord = rightGram;
							cerr << "argmax=" << prefix;
							cerr << "," << bestScore;
							cerr <<" ," << bestWord << endl;
						}				
					}
				}
				else
				{
					double tempScore = lm_->getUnkLogProb() + scores[prefix];
					cerr << "unknown single char word:" << tempScore << endl;
					if(bestScore == 0.0f || tempScore > bestScore)
					{
						bestScore = tempScore;
						bestPrefix = prefix;
						bestWord = rightGram;
						cerr << "argmax=" << prefix;
						cerr << "," << bestScore;
						cerr << "," << bestWord << endl;
					}
				}
			}
		}
		scores[index] = bestScore;
		tracks[index] = bestPrefix;
		words[index] = bestWord;
		cerr << endl << "bestPrefix:" << bestPrefix;
		cerr << ", score=" << bestScore;
		cerr << ", word=" << bestWord << endl;

		if(tracks[index] == -1)	tracks[index] = index - 1;
		
		cerr << endl << "----" << endl;
	}
	
	size_t boundary = length;
	cerr << endl << "backtrack:";
	while(boundary > 0) {
		Token* token = new Token();
		token->word = words[boundary];
		outputTokens.push_back(token);
		boundary = tracks[boundary];
	}
	cerr<<endl;
	reverse(outputTokens.begin(), outputTokens.end());

	return true;
}