#include <algorithm>
#include "tree.h"
#include "evaluator.h"

// �]���\�t�g���J��
void Evaluator::Open()
{
	const auto path = Dialog::GetOpen({ { L"���s�t�@�C�� (*.exe)", L"*.exe" } });

	if (path)
	{
		OpenSub(path.value());
	}
}

// �]���\�t�g�̏����ݒ�I�v�V�������J��
void Evaluator::OpenOption()
{
	const auto path = Dialog::GetOpen({ { L"�e�L�X�g�t�@�C�� (*.txt)", L"*.txt" } });
	if (path)
	{
		mOptions.clear();

		TextReader reader(path.value());
		String line;
		while (reader.readLine(line))
		{
			mOptions.push_back("setoption " + line.narrow()+"\n");
		}
	}

	OpenSub(mEvaluatorPath);
}

void Evaluator::OpenSub(const FilePath& newEvaluatorPath)
{
	// ���݁A�m�[�h��]�����ł���Γǂݎ̂�
	RequestCancel();

	// ���݂̕]���\�t�g������΁A�������̂�I��ł����Ƃ��Ă��A����
	if (mServer != nullptr)
	{
		Close();
	}

	if (newEvaluatorPath != L"")
	{
		mEvaluatorPath = newEvaluatorPath;

		mServer = new Server(mEvaluatorPath, false);

		if (mServer->write("usi\n"))
		{
			// usiok���󂯎��܂ő҂B
			while (!IsUSIOKReceived())
			{
				UpdateReadFromStdio();
			}

			// �]���\�t�g�����󂯎��
			for (const string& s : mReadLogs)
			{
				vector <string> tmp;
				Split1(s, tmp, ' ');
				Trim(tmp.back());

				if (SZ(tmp) >= 3 && tmp[0] == "id" && tmp[1] == "name")
				{
					// �X�y�[�X���͂������]���\�t�g��������̂ŁA�S���q����B
					mName = "";
					for (int i=2;i<SZ(tmp);++i)
					{
						mName += tmp[i];
					}
					break;
				}
			}
			mReadLogs.clear();

			// �I�v�V�����𑗂�
			vector <string> options = mOptions;
			options.push_back("isready\n");
			string allOptions = accumulate(options.begin(), options.end(), string());
			if (mServer->write(allOptions))
			{
				// usiok���󂯎��܂ő҂B
				while (!IsReadyOKReceived())
				{
					UpdateReadFromStdio();
				}
				mReadLogs.clear();

				// �X�^�[�g�I
				if (mServer->write("usinewgame\n"))
				{
				}
				else
				{
					assert(0);
				}
			}
			else
			{
				assert(0);
			}
		}
		else
		{
			assert(0);
		}
	}
}

// �]�����I������
void Evaluator::Close()
{
	if (mServer != nullptr)
	{
		// ���̎��_�œǂݎ̂Ă͏I�����Ă�͂��B
		mServer->write("quit\n");

		delete mServer;
		mServer = nullptr;
	}
}

// ���C�����[�v
bool Evaluator::Update()
{
	if (mServer)
	{

		// �]���̏�ԑJ��
		switch (mEStateEvaluation)
		{
		case EStateEvaluation_FindingNode:
		{
			mEvaludatingNodeID = mTree->GetNextEvaludatedNodeID();
			if (mEvaludatingNodeID != NG)
			{
				if (Go())
				{
					mEStateEvaluation = EStateEvaluation_WaitingScore;
					mStopwatch.restart();
					mPollingStopwatch.restart();
				}
			}
			else
			{
				// �S�m�[�h�̕]�����I��
				return true;
			}
		}
		break;

		case EStateEvaluation_WaitingScore:
			if (UpdateReadFromStdio())
			{
				if (ReceiveBestMoveAndScore())
				{
					mTree->SetFixUpdatedNodeID(mEvaludatingNodeID);
					mEStateEvaluation = EStateEvaluation_FindingNode;
				}
			}
			break;

		default:
			assert(0);
			break;
		}
	}

	return false;
}

// �]�����J�n����
bool Evaluator::Go()
{
	assert(mEvaludatingNodeID != NG);
	const Node& node = mTree->GetNode(mEvaludatingNodeID);

	string writeStr;
	writeStr += "position sfen " + node.mState + "\n";

// 	{
// 		std::wstring wsTmp(writeStr.begin(), writeStr.end());
// 		Print(wsTmp);
// 	}

	assert(mServer != nullptr);

	if (mServer != nullptr && mServer->write(writeStr))
	{
		writeStr = "go btime 0 wtime 0 byoyomi " + to_string(mDurationMilliSec) + "\n";
		return mServer->write(writeStr);
	}

	return false;
}

void Evaluator::RequestCancel()
{
	if (mEStateEvaluation == EStateEvaluation_WaitingScore)
	{
		if (mServer)
		{
			if (mServer->write("stop\n"))
			{
				while (!IsBestMoveReceived())
				{
					UpdateReadFromStdio();
				}
			}
		}
	}

	mEStateEvaluation = EStateEvaluation_FindingNode;
	mReadLogs.clear();
}

// ����I�ɁA�]���\�t�g����A�W�����͌o�R�ŁA�������󂯎�������m�F����B
// �X�V����������true��Ԃ��B
bool Evaluator::UpdateReadFromStdio()
{
	if (mPollingStopwatch.ms() > mDurationMilliSecPolling) // ���Ԍo�߂�����󂯎���
	{
		string readStr;
		if (mServer->read(readStr))
		{
			vector <string> vs;
			Split1(readStr, vs, '\n');

			for(const string& s : vs)
			{
				mReadLogs.push_back(s);
			}
			return true;
		}

		mPollingStopwatch.restart();
	}

	return false;
}


// �]���\�t�g����A�]���l�ƍőP��𓾂�
bool Evaluator::ReceiveBestMoveAndScore()
{
	assert(mEvaludatingNodeID != NG);

	if (!IsBestMoveReceived())
	{
		return false;
	}

	bool isScoreFound = false;

	// bestmove�Ń`�F�b�N���邵���Ȃ��P�[�X
	// �Z�I�F�I�ǎ��ɃX�R�A���܂܂Ȃ��B
	// Apery�F�I�ǎ��̃X�R�A���A�Ȃ���������������(mate 1)
	{
		const int lastIndex = SZ(mReadLogs) - 1;
		if (lastIndex >= 0)
		{
			const string& lastInfo = mReadLogs[lastIndex];

			vector <string> tmp;
			Split1(lastInfo, tmp, ' ');
			Trim(tmp.back());

			assert(tmp[0] == "bestmove");
			if (tmp[1] == "resign")
			{
				isScoreFound = true;
				mTree->UpdateNode(mEvaludatingNodeID, Node::ConvertMateToScore(0), "", true);
			}
		}
	}

	for (int i = SZ(mReadLogs) - 2; i >= 0; --i) // �Ō�̍s��bestmove�Ȃ̂ŁA-2����`�F�b�N�Ő�����
	{
		const string& lastInfo = mReadLogs[i];
// 		std::wstring wsTmp(lastInfo.begin(), lastInfo.end());
// 		Print(wsTmp);

		vector <string> tmp;
		Split1(lastInfo, tmp, ' ');
		Trim(tmp.back());

		int score = 0;
		string tejun;

		assert(tmp[0]=="info");

		bool isJustNowScoreFound = false;

		for (int k = 0; k < SZ(tmp); ++k)
		{
			if (!isScoreFound && tmp[k] == "score" && k + 2 < SZ(tmp))
			{
				if (tmp[k + 1] == "cp")
				{
					score = stoi(tmp[k + 2]);
				}
				else if (tmp[k + 1] == "mate")
				{
					const int mate = stoi(tmp[k + 2]);
					score = Node::ConvertMateToScore(mate);
				}
				isJustNowScoreFound = true;
			}
			else if (tmp[k] == "pv")
			{
				for (int x = k+1; x < SZ(tmp); ++x)
				{
					if (tmp[x] == "None")
					{
						tejun = "";
						break;
					}

					tejun += tmp[x];
					if (x < SZ(tmp) - 1)
					{
						tejun += " ";
					}
				}
			}
			else if (tmp[k] == "nodes")
			{
				mPonderNodes = stoll(tmp[k + 1]);
			}
			else if (tmp[k] == "time")
			{
				mPonderTime = stoll(tmp[k + 1]);
			}
		}

		if (isJustNowScoreFound)
		{
			isScoreFound = true;
			mTree->UpdateNode(mEvaludatingNodeID, score, tejun, false);
			break;
		}
	}

	assert(isScoreFound);

	mReadLogs.clear();
	return true;
}
