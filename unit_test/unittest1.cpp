#include "CppUnitTest.h"
#include "../joseki_viewer/board.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace unit_test
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			// TODO: Your test code here
			Board* board = new Board();

			vector <string> sfen =
			{
				"lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1",
				"ln3gsnl/3rg1kb1/2p2p1p1/pp2p1p1p/4S4/2P1PPP1P/PP1+p1G1P1/1B3RSK1/LN3G1NL b Ps 1",
				"l+R4B1l/2p1g1kG1/2np1sn2/p4pr2/3Pp2pp/P3S4/2N1PPPPP/4GGKS1/L6NL w S4Pb 1",
				"lnsgkgsnl/1r5b1/pppppp1pp/6p2/9/2P6/PP1PPPPPP/1B5R1/LNSGKGSNL b - 1",
				"+B+R1sk1gKl/2G1ss2g/2p4pp/pp4p2/9/7PP/PP2P4/4+s4/2L4NL b RBN7Pg2nl 1",
			};

			for (int i = 0; i < SZ(sfen); ++i)
			{
				board->SetSFEN(sfen[i]);
				string ret = board->GetSFEN();
				Assert::AreEqual(ret, sfen[i]);
			}


			delete board;
		}
	};
}