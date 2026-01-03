#include "CppUnitTest.h"
#include "../joseki_viewer/board.h"
#include "../joseki_viewer/tree.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace unit_test
{		
	TEST_CLASS(UnitTest1)
	{
	public:

		TEST_METHOD(TestTeFromPSN)
		{
			Board* board = new Board();

			struct TejunTeFromPSNCase
			{
				string	tePSN;
				string	te;
			};

			const vector <TejunTeFromPSNCase> cases =
			{
				{ "P7g-7f", "7g7f" },
				{ "P3c-3d", "3c3d" },
				{ "B8hx2b+", "8h2b+" },
				{ "S3ax2b", "3a2b" },
				{ "B*4e", "B*4e" },
				{ "B*5d", "B*5d" },
				{ "B4ex5d", "4e5d" },
				{ "P5cx5d", "5c5d" },
				{ "B*4e", "B*4e" },
				{ "N*7e", "N*7e" },
				{ "+B9i-8i", "9i8i" },
				{ "N7ex8c+", "7e8c+" },
				{ "K8bx8c", "8b8c" },
				{ "+R3ax7a", "3a7a" },
				{ "G*5g", "G*5g" },
				{ "P*4f", "P*4f" },
				{ "G5gx6g", "5g6g" },
				{ "G6hx6g", "6h6g" },
				{ "+B8ix6g", "8i6g" },
				{ "G*6h", "G*6h" },
				{ "+B6gx5f", "6g5f" },
				{ "+R7ax8a", "7a8a" },
			};

			for (const auto& a : cases)
			{
				const string ret = board->GetTeFromPSN(a.tePSN);
				Assert::AreEqual(ret, a.te);
			}

			delete board;
		}

		// 実装せよ！ state + tejun -> next state
		TEST_METHOD(TestMoveByTejun)
		{
			Board* board = new Board();

			struct NextStateCase
			{
				string	state;
				string	tejunPSN;
				string	movedState;
			};

			const vector <NextStateCase> cases =
			{
				{
					"lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
					"P7g-7f P3c-3d G6i-7h P4c-4d K5i-6i R8b-4b B8h-6f K5a-6b S7i-8h K6b-7b S8h-7g K7b-8b P9g-9f P9c-9d K6i-7i S7a-7b K7i-8h S3a-3b P5g-5f S3b-4c S3i-4h S4c-5d B6f-7e P7c-7d B7ex5c+ S5d-6e S4h-5g P4d-4e P6g-6f P4e-4f P4gx4f N8a-7c P6fx6e N7cx6e G4i-5h N6ex7g+ N8ix7g S*6i N*6f S7b-7c G5h-6g S6ix7h K8hx7h G4a-5b +B5c-5d N2a-3c S*6e G*4g S6ex7d S7cx7d N6fx7d K8b-7c S*8b K7cx7d +B5d-3f P6c-6d +B3fx4g K7d-6c +B4g-3f K6c-6b N7g-8e K6b-5a N8e-7c+ N3c-4e P4fx4e B2bx9i+ K7h-6h N*6e S5g-6f L*5c N*4d G5b-4c S*6b G6ax6b +N7cx6b R4bx6b G*7c S*7g S6fx7g N6ex7g+ G6gx7g +B9ix7g K6hx7g N*6e K7g-8f S*7g K8f-7e G*8d",
					"l3k3l/1S1r5/1pG1lg1pp/pg1p1Np2/2Kn1P3/P1P1P1+B2/1Ps3PPP/7R1/7NL b BGSN3Psp 1",
				},

				{
					"lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
					"P2g-2f P3c-3d G6i-7h P4c-4d S3i-4h R8b-4b G4i-5h K5a-6b S7i-6h K6b-7b K5i-6i K7b-8b P3g-3f S7a-7b P1g-1f P9c-9d B8h-7i S3a-3b P5g-5f S3b-4c S6h-5g S4c-5d B7i-6h N2a-3c P4g-4f P4d-4e P4fx4e S5dx4e S5g-4f S4ex3f P*3g P*4g S4hx4g S3fx4g+ G5hx4g N3c-4e P3g-3f G4a-5b P2f-2e B2b-3c P2e-2d P2cx2d P*2e P2dx2e R2hx2e S*3h N2i-3g S3hx4g+ N3gx4e P*2d N4ex3c+ G*5h K6i-7i G5hx6h G7hx6h R4bx4f R2ex2d R4fx5f R2d-2a+ R5f-5i+ G*6i +R5ix1i +R2ax1a L*5d N*4d S*5g N4dx5b+ S5gx6h+ K7ix6h L5d-5g+ K6h-7h +R1ix6i K7h-8h G*7h K8h-9h G*8h",
					"ln1g4+R/1ks1+N4/1pppp1+N1p/p5p2/9/6P1P/PPPP+l+s3/Kgg6/LN1+r5 b BG2SL2Pb3p 1",
				},

				{
					"lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
					"P7g-7f P3c-3d P6g-6f P4c-4d S7i-6h S3a-4b S6h-7g S4b-4c S3i-4h B2b-3c P5g-5f R8b-4b S4h-5g K5a-6b P2g-2f K6b-7b G4i-5h K7b-8b B8h-7i S7a-7b K5i-6h P9c-9d K6h-7h P9d-9e K7h-8h S4c-5d G6i-7h G4a-5b P1g-1f P6c-6d P3g-3f S5d-4e N2i-3g S4ex3f N3g-2e B3c-2d R2h-3h P3d-3e S5g-4f P4d-4e S4fx3e S3f-2g+ R3h-3i P*3g S3ex2d P2cx2d N2e-3c+ N2ax3c P*3d G5b-4c P3dx3c+ G4cx3c P6f-6e P6dx6e R3i-6i R4b-6b N*5e P5c-5d",
					"ln1g4l/1ksr5/1pp3g1p/4p2p1/p2pNp3/2P1P2PP/PPS2Pp+s1/1KG1G4/LNBR4L b Bsn2p 1",
				},

				{
					"lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
					"P7g-7f P3c-3d P6g-6f P4c-4d S7i-6h S3a-4b S6h-6g S4b-4c B8h-7g B2b-3c R2h-8h R8b-4b K5i-4h S7a-7b K4h-3h K5a-6b P8g-8f K6b-7a P8f-8e G4a-5b G6i-5h P4d-4e S6g-5f S4c-5d P7f-7e S5d-5e G4i-4h S5ex6f B7g-8f S6fx7e B8f-7g B3cx7g+ N8ix7g B*3c B*5e B3cx5e S5fx5e B*3c P5g-5f P5c-5d B*3a P5dx5e B3ax4b+ G5bx4b R*3a S*3b N7g-6e S7e-6d R8h-6h P5ex5f R3ax6a+ S7bx6a G*5d S6a-6b L9i-9h B3c-7g+ G5dx6d P6cx6d S*5c S6bx5c N6ex5c+ G4bx5c S*4b G5c-6c S4b-3a+ B*8g +S3ax3b B8gx3b+ S*5b G6c-5c S5b-5a+ S*5g G5hx5g +B7gx6h G5g-5h +B6hx5h G4hx5h +B3b-4b B*6a K7a-6b B6ax3d+ K6bx5a +B3dx4e R*9i P2g-2f S*4i K3h-2g S*5d +B4ex2c S4ix5h+ S3i-3h +S5h-4h +B2c-2b N*3e K2g-3f G*4e K3f-2e G*2d K2e-1f G*1e",
					"ln2k2nl/5+b1+B1/ppp1g3p/3ps2g1/1P3gn1g/4p2PK/P4PP1P/L4+sS2/r6NL b S3Pr3p 1",
				},

				{
					"lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
					"P7g-7f P3c-3d P6g-6f P8c-8d R2h-6h P8d-8e B8h-7g S7a-6b S7i-7h G6a-5b S7h-6g S3a-3b K5i-4h P4c-4d K4h-3h G5b-4c K3h-2h B2b-3c S3i-3h K5a-4b P1g-1f K4b-3a P1f-1e K3a-2b G6i-5h P7c-7d P4g-4f S6b-7c G5h-4g P9c-9d P9g-9f S7c-6d P3g-3f R8b-7b N2i-3g P2c-2d P4f-4e P7d-7e B7g-8h P8e-8f P8gx8f R7b-8b B8h-7g P7ex7f S6gx7f P*7d P6f-6e S6d-7e S7fx7e P7dx7e P4ex4d G4c-4b N3g-4e P7e-7f N4ex3c+ N2ax3c B7g-5e P7f-7g+ B5ex7g P5c-5d B*6f S*5e B6f-7e N*6f S*4c S3bx4c P4dx4c+ G4bx4c P*4d G4c-4b S*4c G4bx4c P4dx4c+ S*3a P5g-5f P*4b +P4cx3c K2bx3c P5fx5e R8b-7b B7ex6f K3c-4c P5ex5d K4c-5b S*5c K5b-6a B6f-8d S*7c N*7d S7cx8d G*6b R7bx6b S5cx6b+",
					"ln1k1gs1l/3+S1p3/3p4p/psN1P1pp1/3P4P/PP4P2/2B2G1P1/3R2SK1/LN3G2L w RN4Pbgp 1",
				},

				{
					"lng3gnl/3sks3/p1ppp1ppp/9/1p1P1p3/2P2S2P/PP2P2P1/3R1P1K1/LN3G1NL b RBGbsp 1",
					"S4fx4e B*4f P*3g B4fx5g+ R6h-9h +B5g-6g G4i-3h +B6gx4e B*7h +B4ex7h R9hx7h B*6g R7h-7i S*6h R7i-3i B6g-5h+ G*4i +B5h-6g R*4g +B6gx8i G4i-5h +B8i-7h B*4e +B7hx4e R4gx4e B*5f R4e-5e B5f-7h+ B*5f +B7hx5f R5ex5f B*7h R5f-6f S6h-7g+ P6e-6d P6cx6d R6fx6d P*6c R6d-8d N*4f G5h-4g N4fx3h+ R3ix3h S*4i R3h-3i G*3h R3ix3h S4ix3h+ K2hx3h +S7g-6g S*8b G7a-6a S8bx8a+ P*4c B*2e R*5e N*4d K5b-4a G*5b G6ax5b N4dx5b+ K4ax5b N*4d K5b-6a G*5b K6a-7b R8d-8b+",
					"l+S4gnl/1+RksGs3/p1ppppppp/5N3/1p2r2B1/2P5P/PP1+s1GPP1/2b2PK2/L6NL w Pgnp 1",
				},

				{
					"ln1g2snl/1ks3rb1/pppgpp1pp/3p5/6p2/1RPP5/P1BSPPPPP/5K1S1/LN1G1G1NL w P 1",
					"P7c-7d G6i-5h N8a-7c K4h-3h P3e-3f P3gx3f R3bx3f P*3g R3f-3b G4i-4h P9c-9d P9g-9f S3a-4b S6g-5f P5c-5d P6f-6e B2bx7g+ N8ix7g P6dx6e N7gx6e N7cx6e B*4f B*7c S5fx6e P*6d N*8e B7c-8d S6ex6d G6cx6d B4fx6d K8b-7a P*6c S*8b G*5c S4bx5c B6dx5c+ K7a-8a +B5cx4c R3b-3c +B4cx5d P*4f P4gx4f N*4b +B5d-4d R3cx6c +B4dx1a R6c-6i+ +B1ax2a N4b-3d +B2a-5d N3dx4f K3h-4g N4fx5h+ G4hx5h +R6i-4i G5h-4h P*4f K4gx4f +R4ix4h K4f-5e +R4h-4g N*7c K8a-9b L*9c S8bx9c N8ex9c+ K9bx9c N*8e K9c-8b N7cx6a+ G*4d +B5dx4d G*6d K5ex6d +R4gx4d S*5d P*6c K6d-6e G*6d K6e-5f +R4dx5d K5f-4g +R5dx5g K4g-3f +R5g-5f K3f-3e +R5f-5e K3e-3f S*2e K3f-4g +R5e-5g K4g-3h +R5g-4h",
					"l2+N5/1ks6/1p1p3pp/pbpg5/1N5s1/PRP6/6PPP/5+rKS1/L6NL b 3GS6Pbnlp 1",
				},

			};


			for (const auto& a : cases)
			{
				board->SetState(a.state);
				const string tejun = board->GetTejunFromPSN(a.tejunPSN);
				board->MoveByTejun(tejun);
				const string ret = board->GetState();
				Assert::AreEqual(ret, a.movedState);
			}

			delete board;
		}

		TEST_METHOD(TestTejunJap)
		{
			Board* board = new Board();

			struct TejunJapCase
			{
				string	state;
				string	tejun;
				wstring	tejunJap;
			};

			const vector <TejunJapCase> cases =
			{
				{
					"lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
					"7g7f",
					L"▲７六歩(77)",
				},

				{
					"+B2gkg2l/2s3r2/ppnp1p1pb/2R3p1p/3P5/6s1P/PP1SPPNP1/4GGK2/LN4S1L b NLP3p 1",
					"L*4f 4a4b N*5e 1c4f 4g4f 3d3e P*3c 3b3c B*2b 3f3g+ 3h3g 3e3f 3g4g 3c3e 9a7c 7b7c 7d7c+ B*9a",
					L"▲４六香打△４二金(41)▲５五桂打△４六角(13)▲同歩(47)△３五歩(34)▲３三歩打△同飛(32)▲２二角打△３七銀成(36)▲同玉(38)△３六歩(35)▲４七玉(37)△３五飛(33)▲７三馬(91)△同銀(72)▲同飛成(74)△９一角打",
				},


				{
					"+B7l/2s1+R2k1/pp1p2Ppb/6p1p/3n5/8P/PP1SPPGP1/4G1K2/LN4S1L w RGSNPgnl4p 1",
					"2b3c N*2e 3c2d S*3c 2d2e G*2f",
					L"△３三玉(22)▲２五桂打△２四玉(33)▲３三銀打△２五玉(24)▲２六金打",
				},

				{
					"ln6l/1skgg2B1/pppppp2p/5s3/P1r6/5P3/1PPPP+BPPP/1S2G1KS1/LNG4NL b Nr2p 1",
					"2b1a+ 7e7d 5h4h 7d6d 1a1b 4d5e 3g3f P*3d 7i6h 6d4d 2h3g 4d6d 5g5f R*5i 5f5e 5i8i+ S*7i N*2e 3g2f",
					L"▲１一角成(22)△７四飛(75)▲４八金(58)△６四飛(74)▲１二馬(11)△５五銀(44)▲３六歩(37)△３四歩打▲６八金(79)△４四飛(64)▲３七銀(28)△６四飛(44)▲５六歩(57)△５九飛打▲５五歩(56)△８九飛成(59)▲７九銀打△２五桂打▲２六銀(37)",
				},

				{
					"9/4+S4/1+S1+P+S+P1+Pp/4P1p2/p8/2+B3P2/PP1G3+n1/LSG1+p+lg2/KNP3k2 w GNL5P2rbnlp 1",
					"R*1f 7f7g P*6h 7h6h 5h6h 7g6h N*7e 6g7g L*8a L*8f 1f1h+ 2c1c 8a8c 8f8c+",
					L"△１六飛打▲７七馬(76)△６八歩打▲同金(78)△同と(58)▲同馬(77)△７五桂打▲７七金(67)△８一香打▲８六香打△１八飛成(16)▲１三と(23)△８三香(81)▲同香成(86)",
				},

				{
					"ln1g3+B1/1ks1g4/1pppp2pp/p5p2/7P1/2P4R1/PP+BPP1P1P/LSG2+p3/KNG3+rNL w SNLsp 1",
					"4h5h 2a6e 5h6i 7i6i 3i6i 6e5f 6i5h L*8f G*4g 7h6h 5h4i N*7e S*7d 2e2d 4g5h 6h5h 4i5h G*5i",
					L"△５八と(48)▲６五馬(21)△６九と(58)▲同金(79)△同龍(39)▲５六馬(65)△５八龍(69)▲８六香打△４七金打▲６八金(78)△４九龍(58)▲７五桂打△７四銀打▲２四歩(25)△５八金(47)▲同金(68)△同龍(49)▲５九金打",
				},
			};


			for (const auto& a : cases)
			{
				board->SetState(a.state);
				const wstring ret = board->MoveByTejun(a.tejun);
				Assert::AreEqual(ret, a.tejunJap);
			}

			delete board;
		}

		TEST_METHOD(TestState)
		{
			Board* board = new Board();

			vector <string> state =
			{
				"lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
				"ln3gsnl/3rg1kb1/2p2p1p1/pp2p1p1p/4S4/2P1PPP1P/PP1+p1G1P1/1B3RSK1/LN3G1NL b Ps 1",
				"l+R4B1l/2p1g1kG1/2np1sn2/p4pr2/3Pp2pp/P3S4/2N1PPPPP/4GGKS1/L6NL w S4Pb 1",
				"lnsgkgsnl/1r5b1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/1B5R1/LNSGKGSNL b - 1",
				"+B+R1sk1gKl/2G1ss2g/2p4pp/pp4p2/9/7PP/PP2P4/4+s4/2L4NL b RBN7Pg2nl 1",
			};

			for (int i = 0; i < SZ(state); ++i)
			{
				board->SetState(state[i]);
				string ret = board->GetState();
				Assert::AreEqual(ret, state[i]);
			}

			delete board;
		}

		TEST_METHOD(TestMoveFromTe)
		{
			Board* board = new Board();

			const vector <string> tes =
			{
				"7g7f",
				"3c3d",
				"8h2b+",
				"3a2b",
				"B*4e",
				"B*5d",
				"4e5d",
				"5c5d",
				"B*4e",
				"N*7e",
				"9i8i",
				"7e8c+",
				"8b8c",
				"3a7a",
				"G*5g",
				"P*4f",
				"5g6g",
				"6h6g",
				"8i6g",
				"G*6h",
				"6g5f",
				"7a8a",
			};

			for (const auto& te : tes)
			{
				Move mv = board->GetMoveFromTe(te);
				string ret = board->GetTeFromMove(mv);
				Assert::AreEqual(ret, te);
			}

			delete board;
		}

		TEST_METHOD(TestTreeAddLink)
		{
			Board* board = new Board();
			Tree* tree = new Tree(board);

			tree->AddLink("7g7f");
			tree->AddLink("3c3d");
			tree->AddLink("6i7h");

			tree->SetSelectedNodeID(0);
			tree->AddLink("8c8d");
			tree->AddLink("6i7h");
			tree->CalculateVisualPos();

			Assert::AreEqual(tree->GetNode(0).mVisualX, 0.f);
			Assert::AreEqual(tree->GetNode(0).mVisualY, 0.f);

			Assert::AreEqual(tree->GetNode(1).mVisualX, -1.f);
			Assert::AreEqual(tree->GetNode(1).mVisualY, 1.f);

			Assert::AreEqual(tree->GetNode(2).mVisualX, -1.f);
			Assert::AreEqual(tree->GetNode(2).mVisualY, 2.f);

			Assert::AreEqual(tree->GetNode(3).mVisualX, -1.f);
			Assert::AreEqual(tree->GetNode(3).mVisualY, 3.f);

			Assert::AreEqual(tree->GetNode(4).mVisualX, 1.f);
			Assert::AreEqual(tree->GetNode(4).mVisualY, 1.f);

			Assert::AreEqual(tree->GetNode(5).mVisualX, 1.f);
			Assert::AreEqual(tree->GetNode(5).mVisualY, 2.f);

			delete tree;
			delete board;
		}

		TEST_METHOD(TestKifToTejun)
		{
			struct KifToTejunCase
			{
				vector <wstring> kifStrings;
				string	truthState;
			};

			vector <KifToTejunCase> casesKifToTejun = 
			{
				// Test 0
				{
					{
						L"開始日時：2016/11/26 07:28:10",
						L"棋戦：将棋ウォーズ(10切)",
						L"持ち時間：10分切れ負け",
						L"手合割：平手",
						L"先手：Felisidade",
						L"後手：shindannin",
						L"手数----指手---------消費時間--",
						L"1 ７六歩(77)   ( 00:00/00:00:00)",
						L"2 ３四歩(33)   ( 00:00/00:00:00)",
						L"3 ７五歩(76)   ( 00:01/00:00:01)",
						L"4 ４四歩(43)   ( 00:02/00:00:02)",
						L"5 ７八飛(28)   ( 00:02/00:00:03)",
						L"6 ４二銀(31)   ( 00:01/00:00:03)",
						L"7 ７四歩(75)   ( 00:01/00:00:04)",
						L"8 同　歩(73)   ( 00:03/00:00:06)",
						L"9 ５五角(88)   ( 00:09/00:00:13)",
						L"10 ９二飛(82)   ( 00:22/00:00:28)",
						L"11 ７四飛(78)   ( 00:04/00:00:17)",
						L"12 ７三歩打   ( 00:03/00:00:31)",
						L"13 ７八飛(74)   ( 00:02/00:00:19)",
						L"14 ４三銀(42)   ( 00:06/00:00:37)",
						L"15 ８八角(55)   ( 00:10/00:00:29)",
						L"16 ３三角(22)   ( 00:10/00:00:47)",
						L"17 ４八玉(59)   ( 00:02/00:00:31)",
						L"18 ２二飛(92)   ( 00:03/00:00:50)",
						L"19 ３八玉(48)   ( 00:02/00:00:33)",
						L"20 ２四歩(23)   ( 00:11/00:01:01)",
						L"21 ９六歩(97)   ( 00:05/00:00:38)",
						L"22 ２五歩(24)   ( 00:06/00:01:07)",
						L"23 ９七角(88)   ( 00:04/00:00:42)",
						L"24 ５二金(41)   ( 00:02/00:01:09)",
						L"25 ４八金(49)   ( 00:10/00:00:52)",
						L"26 ２六歩(25)   ( 00:05/00:01:14)",
						L"27 同　歩(27)   ( 00:03/00:00:55)",
						L"28 同　飛(22)   ( 00:01/00:01:15)",
						L"29 ２七歩打   ( 00:02/00:00:57)",
						L"30 ２四飛(26)   ( 00:01/00:01:16)",
						L"31 ２八銀(39)   ( 00:04/00:01:01)",
						L"32 ４五歩(44)   ( 00:08/00:01:24)",
						L"33 ８八角(97)   ( 00:03/00:01:04)",
						L"34 ６二玉(51)   ( 00:16/00:01:40)",
						L"35 ２六歩(27)   ( 00:21/00:01:25)",
						L"36 同　飛(24)   ( 00:14/00:01:54)",
						L"37 ２七銀(28)   ( 00:05/00:01:30)",
						L"38 ２四飛(26)   ( 00:04/00:01:58)",
						L"39 ２六歩打   ( 00:03/00:01:33)",
						L"40 ７二玉(62)   ( 00:26/00:02:24)",
						L"41 ３六歩(37)   ( 00:03/00:01:36)",
						L"42 ６二金(61)   ( 00:03/00:02:27)",
						L"43 ３七桂(29)   ( 00:03/00:01:39)",
						L"44 ５四銀(43)   ( 00:07/00:02:34)",
						L"45 ２五歩(26)   ( 00:08/00:01:47)",
						L"46 ２二飛(24)   ( 00:06/00:02:40)",
						L"47 ２六銀(27)   ( 00:13/00:02:00)",
						L"48 ８八角成(33)   ( 00:05/00:02:45)",
						L"49 同　銀(79)   ( 00:02/00:02:02)",
						L"50 ４六歩(45)   ( 00:48/00:03:33)",
						L"51 同　歩(47)   ( 00:26/00:02:28)",
						L"52 ７四角打   ( 00:08/00:03:41)",
						L"53 ４七金(48)   ( 00:07/00:02:35)",
						L"54 ４二飛(22)   ( 00:07/00:03:48)",
						L"55 ５六歩(57)   ( 00:21/00:02:56)",
						L"56 ３三桂(21)   ( 00:16/00:04:04)",
						L"57 ２四角打   ( 00:20/00:03:16)",
						L"58 ４三飛(42)   ( 00:18/00:04:22)",
						L"59 ７五歩打   ( 00:13/00:03:29)",
						L"60 ８五角(74)   ( 00:07/00:04:29)",
						L"61 ５八金(69)   ( 00:05/00:03:34)",
						L"62 ６五銀(54)   ( 00:25/00:04:54)",
						L"63 ４五歩(46)   ( 00:12/00:03:46)",
						L"64 ７六銀(65)   ( 00:10/00:05:04)",
						L"65 ５七金(47)   ( 00:11/00:03:57)",
						L"66 ４五桂(33)   ( 00:10/00:05:14)",
						L"67 同　桂(37)   ( 00:07/00:04:04)",
						L"68 同　飛(43)   ( 00:01/00:05:15)",
						L"69 ４六角(24)   ( 00:19/00:04:23)",
						L"70 同　飛(45)   ( 00:09/00:05:24)",
						L"71 同　金(57)   ( 00:04/00:04:27)",
						L"72 ６七銀成(76)   ( 00:03/00:05:27)",
						L"73 ７九飛(78)   ( 00:08/00:04:35)",
						L"74 ５八成銀(67)   ( 00:10/00:05:37)",
						L"75 ２七玉(38)   ( 00:05/00:04:40)",
						L"76 ４九角打   ( 00:26/00:06:03)",
						L"77 ３八桂打   ( 00:05/00:04:45)",
						L"78 ４八成銀(58)   ( 00:11/00:06:14)",
						L"79 １六玉(27)   ( 00:06/00:04:51)",
						L"80 ３八角成(49)   ( 00:02/00:06:16)",
						L"81 ２七飛打   ( 00:02/00:04:53)",
						L"82 １四歩(13)   ( 00:36/00:06:52)",
						L"83 ２四歩(25)   ( 00:09/00:05:02)",
						L"84 ２七馬(38)   ( 00:53/00:07:45)",
						L"85 同　玉(16)   ( 00:03/00:05:05)",
						L"86 ６七飛打   ( 00:04/00:07:49)",
						L"87 ３七銀(26)   ( 00:12/00:05:17)",
						L"88 ５七飛成(67)   ( 00:07/00:07:56)",
						L"89 １六歩(17)   ( 00:16/00:05:33)",
						L"90 ４九角成(85)   ( 00:14/00:08:10)",
						L"91 ２六玉(27)   ( 00:06/00:05:39)",
						L"92 ３八馬(49)   ( 00:12/00:08:22)",
						L"93 ４八銀(37)   ( 00:13/00:05:52)",
						L"94 同　馬(38)   ( 00:03/00:08:25)",
						L"95 ２五玉(26)   ( 00:05/00:05:57)",
						L"96 ４六龍(57)   ( 00:06/00:08:31)",
						L"97 ３四玉(25)   ( 00:02/00:05:59)",
						L"98 ４三龍(46)   ( 00:04/00:08:35)",
						L"99 ２五玉(34)   ( 00:02/00:06:01)",
						L"100 ３四銀打   ( 00:05/00:08:40)",
						L"101 投了",
					},

					"lns5l/2kgg4/ppppp+r3/6sPp/2P4K1/P3P1P1P/1P7/1S3+b3/LNR5L b BSP2g2n4p 1",
				}, 

				// Test 1
				{
					{
						L"開始日時：2016/10/10 20:13:15",
						L"棋戦：将棋ウォーズ(10切)",
						L"持ち時間：10分切れ負け",
						L"手合割：平手",
						L"先手：takashiyao0522",
						L"後手：shindannin",
						L"手数----指手---------消費時間--",
						L"1 ７六歩(77)   ( 00:00/00:00:00)",
						L"2 ３四歩(33)   ( 00:01/00:00:01)",
						L"3 ６八飛(28)   ( 00:03/00:00:03)",
						L"4 ４四歩(43)   ( 00:02/00:00:03)",
						L"5 ４八玉(59)   ( 00:02/00:00:05)",
						L"6 ４二銀(31)   ( 00:01/00:00:04)",
						L"7 ３八玉(48)   ( 00:01/00:00:06)",
						L"8 ４三銀(42)   ( 00:01/00:00:05)",
						L"9 ２八玉(38)   ( 00:02/00:00:08)",
						L"10 ３三角(22)   ( 00:06/00:00:11)",
						L"11 １八香(19)   ( 00:02/00:00:10)",
						L"12 ２二飛(82)   ( 00:05/00:00:16)",
						L"13 １九玉(28)   ( 00:02/00:00:12)",
						L"14 ６二玉(51)   ( 00:02/00:00:18)",
						L"15 ２八銀(39)   ( 00:02/00:00:14)",
						L"16 ７二玉(62)   ( 00:02/00:00:20)",
						L"17 ３九金(49)   ( 00:04/00:00:18)",
						L"18 ５二金(41)   ( 00:01/00:00:21)",
						L"19 ５九金(69)   ( 00:00/00:00:18)",
						L"20 ８二玉(72)   ( 00:01/00:00:22)",
						L"21 ４九金(59)   ( 00:01/00:00:19)",
						L"22 ９二香(91)   ( 00:01/00:00:23)",
						L"23 ３八金(49)   ( 00:01/00:00:20)",
						L"24 ９一玉(82)   ( 00:00/00:00:23)",
						L"25 ９六歩(97)   ( 00:00/00:00:20)",
						L"26 ８二銀(71)   ( 00:02/00:00:25)",
						L"27 ９五歩(96)   ( 00:01/00:00:21)",
						L"28 ７一金(61)   ( 00:01/00:00:26)",
						L"29 ７七角(88)   ( 00:03/00:00:24)",
						L"30 ６二金(52)   ( 00:01/00:00:27)",
						L"31 ６六歩(67)   ( 00:01/00:00:25)",
						L"32 ５四銀(43)   ( 00:07/00:00:34)",
						L"33 ６五歩(66)   ( 00:02/00:00:27)",
						L"34 ７二金(62)   ( 00:05/00:00:39)",
						L"35 ９八香(99)   ( 00:02/00:00:29)",
						L"36 ６二飛(22)   ( 00:01/00:00:40)",
						L"37 ７八銀(79)   ( 00:01/00:00:30)",
						L"38 １二香(11)   ( 00:02/00:00:42)",
						L"39 ６七銀(78)   ( 00:03/00:00:33)",
						L"40 ４五歩(44)   ( 00:03/00:00:45)",
						L"41 ６六銀(67)   ( 00:03/00:00:36)",
						L"42 １四歩(13)   ( 00:13/00:00:58)",
						L"43 ７五銀(66)   ( 00:14/00:00:50)",
						L"44 ７七角成(33)   ( 00:08/00:01:06)",
						L"45 同　桂(89)   ( 00:01/00:00:51)",
						L"46 ３三角打   ( 00:23/00:01:29)",
						L"47 ６六角打   ( 01:03/00:01:54)",
						L"48 同　角(33)   ( 00:06/00:01:35)",
						L"49 同　銀(75)   ( 00:03/00:01:57)",
						L"50 ３三角打   ( 00:38/00:02:13)",
						L"51 ９四歩(95)   ( 00:22/00:02:19)",
						L"52 同　歩(93)   ( 00:05/00:02:18)",
						L"53 同　香(98)   ( 00:27/00:02:46)",
						L"54 同　香(92)   ( 00:15/00:02:33)",
						L"55 ８五桂(77)   ( 00:06/00:02:52)",
						L"56 ８四歩(83)   ( 00:27/00:03:00)",
						L"57 ６四歩(65)   ( 00:10/00:03:02)",
						L"58 ８五歩(84)   ( 00:11/00:03:11)",
						L"59 ６三歩成(64)   ( 00:02/00:03:04)",
						L"60 同　銀(54)   ( 00:02/00:03:13)",
						L"61 ７五銀(66)   ( 00:06/00:03:10)",
						L"62 ７七角成(33)   ( 00:13/00:03:26)",
						L"63 ６五飛(68)   ( 00:03/00:03:13)",
						L"64 ７六馬(77)   ( 00:08/00:03:34)",
						L"65 ４五飛(65)   ( 00:06/00:03:19)",
						L"66 ４三香打   ( 00:39/00:04:13)",
						L"67 ２五飛(45)   ( 00:13/00:03:32)",
						L"68 ２二飛(62)   ( 00:12/00:04:25)",
						L"69 ６四歩打   ( 00:15/00:03:47)",
						L"70 ５四銀(63)   ( 00:17/00:04:42)",
						L"71 ３一角打   ( 00:18/00:04:05)",
						L"72 ５二飛(22)   ( 00:45/00:05:27)",
						L"73 ２三飛成(25)   ( 00:05/00:04:10)",
						L"74 ７五馬(76)   ( 00:09/00:05:36)",
						L"75 ２一龍(23)   ( 00:14/00:04:24)",
						L"76 ６四馬(75)   ( 00:07/00:05:43)",
						L"77 ５六桂打   ( 00:20/00:04:44)",
						L"78 ５五馬(64)   ( 00:21/00:06:04)",
						L"79 ６五歩打   ( 00:05/00:04:49)",
						L"80 ６六歩打   ( 00:23/00:06:27)",
						L"81 ６四歩(65)   ( 00:15/00:05:04)",
						L"82 ６七歩成(66)   ( 00:10/00:06:37)",
						L"83 ６三歩成(64)   ( 00:55/00:05:59)",
						L"84 同　金(72)   ( 00:03/00:06:40)",
						L"85 ８六歩(87)   ( 00:16/00:06:15)",
						L"86 ５七と(67)   ( 00:15/00:06:55)",
						L"87 ６四歩打   ( 00:02/00:06:17)",
						L"88 ７四金(63)   ( 00:13/00:07:08)",
						L"89 ６三歩成(64)   ( 00:05/00:06:22)",
						L"90 同　銀(54)   ( 00:07/00:07:15)",
						L"91 ６四歩打   ( 00:35/00:06:57)",
						L"92 ７二銀(63)   ( 00:04/00:07:19)",
						L"93 ８五歩(86)   ( 00:26/00:07:23)",
						L"94 ５六と(57)   ( 00:05/00:07:24)",
						L"95 ８四歩(85)   ( 00:05/00:07:28)",
						L"96 同　金(74)   ( 00:05/00:07:29)",
						L"97 ８三歩打   ( 00:18/00:07:46)",
						L"98 同　金(84)   ( 00:04/00:07:33)",
						L"99 ６三歩成(64)   ( 00:05/00:07:51)",
						L"100 同　銀(72)   ( 00:04/00:07:37)",
						L"101 ５三角成(31)   ( 00:13/00:08:04)",
						L"102 同　飛(52)   ( 00:04/00:07:41)",
						L"103 １二龍(21)   ( 00:02/00:08:06)",
						L"104 ４七と(56)   ( 00:09/00:07:50)",
						L"105 １四龍(12)   ( 00:08/00:08:14)",
						L"106 ５六馬(55)   ( 00:06/00:07:56)",
						L"107 ５九香打   ( 00:12/00:08:26)",
						L"108 ５七歩打   ( 00:03/00:07:59)",
						L"109 １六龍(14)   ( 00:05/00:08:31)",
						L"110 ４六歩打   ( 00:02/00:08:01)",
						L"111 ５四歩打   ( 00:03/00:08:34)",
						L"112 同　銀(63)   ( 00:03/00:08:04)",
						L"113 ５七香(59)   ( 00:02/00:08:36)",
						L"114 同　馬(56)   ( 00:03/00:08:07)",
						L"115 投了",
						L"",
						L"",
					},

					"kng6/1s7/1gp1rl3/l3s1p2/9/5p2+R/4+b+pPPP/6GSL/6GNK b 2Pbs2nl9p 1",
				}
			};

			Board* board = new Board();
			Tree* tree = new Tree(board);

			for (const KifToTejunCase& a : casesKifToTejun)
			{
				board->Init();

				const string tejun = board->GetTejunFromKif(a.kifStrings);
				board->MoveByTejun(tejun);
				const string ret = board->GetState();
				Assert::AreEqual(ret, a.truthState);
			}

			delete tree;
			delete board;
		}
	};
}