#include <algorithm>
#include "tree.h"
#include "evaluator.h"

// 評価ソフトを開く
void Evaluator::Open()
{
	const auto path = Dialog::GetOpen({ { L"実行ファイル (*.exe)", L"*.exe" } });

	if (path)
	{
		OpenSub(path.value());
	}
}

// 評価ソフトの初期設定オプションを開く
void Evaluator::OpenOption()
{
	const auto path = Dialog::GetOpen({ { L"テキストファイル (*.txt)", L"*.txt" } });
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
	// 現在、ノードを評価中であれば読み捨て
	RequestCancel();

	// 現在の評価ソフトがあれば、同じものを選んでいたとしても、閉じる
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
			// usiokを受け取るまで待つ。
			while (!IsUSIOKReceived())
			{
				UpdateReadFromStdio();
			}

			// 評価ソフト名を受け取る
			for (const string& s : mReadLogs)
			{
				vector <string> tmp;
				Split1(s, tmp, ' ');
				Trim(tmp.back());

				if (SZ(tmp) >= 3 && tmp[0] == "id" && tmp[1] == "name")
				{
					// スペースがはいった評価ソフト名もあるので、全部繋げる。
					mName = "";
					for (int i=2;i<SZ(tmp);++i)
					{
						mName += tmp[i];
					}
					break;
				}
			}
			mReadLogs.clear();

			// オプションを送る
			vector <string> options = mOptions;
			options.push_back("isready\n");
			string allOptions = accumulate(options.begin(), options.end(), string());
			if (mServer->write(allOptions))
			{
				// usiokを受け取るまで待つ。
				while (!IsReadyOKReceived())
				{
					UpdateReadFromStdio();
				}
				mReadLogs.clear();

				// スタート！
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

// 評価を終了する
void Evaluator::Close()
{
	if (mServer != nullptr)
	{
		// この時点で読み捨ては終了してるはず。
		mServer->write("quit\n");

		delete mServer;
		mServer = nullptr;
	}
}

// メインループ
bool Evaluator::Update()
{
	if (mServer)
	{

		// 評価の状態遷移
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
				// 全ノードの評価が終了
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

// 評価を開始する
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

// 定期的に、評価ソフトから、標準入力経由で、何かを受け取ったか確認する。
// 更新があったらtrueを返す。
bool Evaluator::UpdateReadFromStdio()
{
	if (mPollingStopwatch.ms() > mDurationMilliSecPolling) // 時間経過したら受け取りに
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


// 評価ソフトから、評価値と最善手を得る
bool Evaluator::ReceiveBestMoveAndScore()
{
	assert(mEvaludatingNodeID != NG);

	if (!IsBestMoveReceived())
	{
		return false;
	}

	bool isScoreFound = false;

	// bestmoveでチェックするしかないケース
	// 技巧：終局時にスコアを含まない。
	// Apery：終局時のスコアが、なぜか自分が勝ちに(mate 1)
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

	for (int i = SZ(mReadLogs) - 2; i >= 0; --i) // 最後の行はbestmoveなので、-2からチェックで正しい
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
