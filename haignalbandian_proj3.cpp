#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <dirent.h>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

using namespace std;

vector<string> getAllFilesInDirectory(string path){

	DIR*    dir;
    dirent* pdir;
    std::vector<std::string> files;

    dir = opendir(path.c_str());

    while (pdir = readdir(dir)) {
        files.push_back(path+pdir->d_name);
    }
    
    return files;
}

class Email{
	public:
		Email(string fedPath, const unordered_set<string>& wordList){
			path = fedPath;
			wordCount = new unordered_map<string,int>();
			for(auto it: wordList){
				wordCount->emplace(it,0);
			}

			ifstream email(path);
			string word;
			if(email.is_open()){
		
				while(email>>word){
					auto wordIt = wordCount->find(word);
					if(wordIt == wordCount->end()){
						continue;
					}else if(wordCount->at(word)==3){
						continue;
					}else{
						wordCount->at(word)++;
					}

				}

			email.close();
			}

		}

		int getWordCount(string word){
			if(wordCount->find(word)==wordCount->end()){
				return -1;
			}
			return wordCount->at(word);
		}

		string getPath(){
			return path;
		}
	private:
		string path;
		unordered_map<string,int>* wordCount;
};

void testTwoSets(const unordered_set<string>& wordList, const unordered_map<string,vector<vector<int>>>& learnedProbabilities, const vector<Email*>& testingSpamEmails, const vector<Email*>& testingHamEmails, double probabilityOfSpam, double probabilityOfHam, int trainingSpamSize, int trainingHamSize){

	int classifiedSpam=0, classifiedHam=0, classifiedSpamCorrectly=0;
	
	for(int i=0; i<testingSpamEmails.size(); i++){
		double spamWords=0, hamWords=0;
		for(auto it: wordList){
			
			if(learnedProbabilities.at(it)[0][testingSpamEmails[i]->getWordCount(it)]==0){
				spamWords+=-100;
			}else{
				double value = log((double)learnedProbabilities.at(it)[0][testingSpamEmails[i]->getWordCount(it)]/(double)trainingSpamSize);

				spamWords+= value;
			}
			if((double)learnedProbabilities.at(it)[1][testingSpamEmails[i]->getWordCount(it)]==0){
				hamWords+=-100;
			}else{
				hamWords+= log((double)learnedProbabilities.at(it)[1][testingSpamEmails[i]->getWordCount(it)]/(double)trainingHamSize);
			}
		}
		
		spamWords+= log(probabilityOfSpam);
		hamWords+= log(probabilityOfHam);
		if(spamWords>hamWords){
			classifiedSpam++;
			classifiedSpamCorrectly++;
		}else{
			classifiedHam++;
		}
	}

	

	for(int i=0; i<testingHamEmails.size(); i++){
		double spamWords=0, hamWords=0;
		for(auto it: wordList){
			if(learnedProbabilities.at(it)[0][testingHamEmails[i]->getWordCount(it)]==0){
				spamWords+=-100;
			}else{
				spamWords+= log((double)learnedProbabilities.at(it)[0][testingHamEmails[i]->getWordCount(it)]/(double)trainingSpamSize);
			}
			if((double)learnedProbabilities.at(it)[1][testingHamEmails[i]->getWordCount(it)]==0){
				hamWords+=-100;
			}else{
				hamWords+= log((double)learnedProbabilities.at(it)[1][testingHamEmails[i]->getWordCount(it)]/(double)trainingHamSize);
			}
		}
		spamWords+= log(probabilityOfSpam);
		hamWords+= log(probabilityOfHam);
		if(spamWords>hamWords){
			cout<<"Misclassified as spam: "<<testingHamEmails[i]->getPath()<<endl;
			classifiedSpam++;
		}else{
			classifiedHam++;
		}
	}
	cout<<endl<<"Classified Correctly Spam "<<classifiedSpamCorrectly<<endl;
	cout<<"Classified Spam "<<classifiedSpam<<endl;
	cout<<"Classified Ham "<<classifiedHam<<endl<<endl;
	
	double precision = (double)classifiedSpamCorrectly/(double)classifiedSpam;
	double recall = (double)classifiedSpamCorrectly/(double)testingSpamEmails.size();

	cout<<"PRECISION: "<<precision<<endl;
	cout<<"RECALL: "<<recall<<endl<<endl;;
}

int main(){
	
	unordered_set<string> wordList;

	ifstream dicFile("./data/dict.txt");
	string line;
	while(dicFile>>line){
		wordList.emplace(line);
	}

	vector<string> trainingSpam(getAllFilesInDirectory("./data/train/spam/")), 
		trainingHam(getAllFilesInDirectory("./data/train/ham/")), 
		testingSpam(getAllFilesInDirectory("./data/test/spam/")), 
		testingHam(getAllFilesInDirectory("./data/test/ham/"));
	
	vector<Email*> trainingSpamEmails, trainingHamEmails, testingSpamEmails, testingHamEmails;

	for(int i=0; i<trainingSpam.size(); i++){
		if(trainingSpam[i]=="./data/train/spam/."||trainingSpam[i]=="./data/train/spam/.."){
			continue;
		}
		trainingSpamEmails.push_back(new Email(trainingSpam[i],wordList));
	}
	for(int i=0; i<trainingHam.size(); i++){
		if(trainingHam[i]=="./data/train/ham/."||trainingHam[i]=="./data/train/ham/.."){
			continue;
		}
		trainingHamEmails.push_back(new Email(trainingHam[i],wordList));
	}

	double probabilityOfSpam = (double)trainingSpamEmails.size()/(double)(trainingHamEmails.size()+trainingSpamEmails.size());
	double probabilityOfHam = (double)trainingHamEmails.size()/(double)(trainingHamEmails.size()+trainingSpamEmails.size());
	
	unordered_map<string, vector<vector<int> > > learnedProbabilities;
	for(auto it: wordList){
		vector< vector<int> > emptyVector(2,vector<int>(4));
		for(int i=0; i<2; i++){
			for(int j=0; j<4; j++){
				emptyVector[i][j]=0;
			}
		}
		learnedProbabilities.emplace(it,emptyVector);
	}
	
	for(auto it: wordList){
		for(int i=0; i<trainingSpamEmails.size(); i++){

			learnedProbabilities.at(it)[0][trainingSpamEmails[i]->getWordCount(it)]++;
			
		}
		for(int i=0; i<trainingHamEmails.size(); i++){
			learnedProbabilities.at(it)[1][trainingHamEmails[i]->getWordCount(it)]++;
		}
	}

	for(int i=0; i<testingSpam.size(); i++){
		if(testingSpam[i]=="./data/test/spam/."||testingSpam[i]=="./data/test/spam/.."){
			continue;
		}
		testingSpamEmails.push_back(new Email(testingSpam[i],wordList));
	}
	for(int i=0; i<testingHam.size(); i++){
		if(testingHam[i]=="./data/test/ham/."||testingHam[i]=="./data/test/ham/.."){
			continue;
		}
		testingHamEmails.push_back(new Email(testingHam[i],wordList));
	}
	cout<<endl<<"TESTING SET RESULTS: "<<endl<<endl;
	testTwoSets(wordList,learnedProbabilities,testingSpamEmails,testingHamEmails,probabilityOfSpam,probabilityOfHam,trainingSpamEmails.size(),trainingHamEmails.size());
	

	cout<<endl<<endl<<endl<<"TRAINING SET RESULTS: "<<endl<<endl;
	testTwoSets(wordList,learnedProbabilities,trainingSpamEmails,trainingHamEmails,probabilityOfSpam,probabilityOfHam,trainingSpamEmails.size(),trainingHamEmails.size());
	
	int maxSpam=0,  maxHam=0;
	string maxSpamString, maxHamString;
	cout<<endl<<endl<<"MOST LIKELY WORDS (ignore first few)"<<endl;
	for (auto it: learnedProbabilities){
		int calcSpam = it.second[0][1]+2*it.second[0][2]+3*it.second[0][3];
		int calcHam = it.second[1][1]+2*it.second[1][2]+3*it.second[1][3];
		if(calcSpam>maxSpam){
			maxSpam = calcSpam;
			maxSpamString = it.first;
			cout<<"SPAM   "<<maxSpamString<<"   "<<maxSpam<<endl;

		}
		if(calcHam>maxHam){
			maxHam = calcHam;
			maxHamString = it.first;
			cout<<"HAM "<<maxHamString<<"   "<<maxHam<<endl;
		}
	}

	cout<<endl<<"Most likely spam word: "<<maxSpamString<<endl;
	cout<<"Most likely ham word: "<<maxHamString<<endl<<endl<<endl;
	
	return 0;

}