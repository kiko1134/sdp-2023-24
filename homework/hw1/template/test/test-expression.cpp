#include "catch2/catch_all.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "expression-lib/expression.h"


// Ensures (with REQUIRE) that an expression evaluates to a given value
void requireExpressionEvaluatesTo(const char* expression, std::istream& ops, double expectedValue)
{
	double result = evaluate(expression, ops);
 	REQUIRE_THAT(result, Catch::Matchers::WithinRel(expectedValue, 0.001));	
}

TEST_CASE("evaluate() returns 0 for the empty string")
{
	std::stringstream empty;
	requireExpressionEvaluatesTo("", empty, 0.0);
}

TEST_CASE("evaluate() returns 0 for a string of space characters")
{
	std::stringstream empty;
	requireExpressionEvaluatesTo("   ", empty, 0.0);
}

// Ensures (with REQUIRE) that evaluate correctly detects the expression as incorrect.
void requireIncorrectExpressionDetection(const char* expression, std::istream& ops)
{
	REQUIRE_THROWS_AS(evaluate(expression, ops), incorrect_expression);
}

TEST_CASE("evaluate() throws when expression is nullptr")
{
	std::stringstream empty;
	requireIncorrectExpressionDetection(nullptr, empty);
}

TEST_CASE("evaluate() identifies incorrect expressions and throws the required exception")
{
	std::stringstream ops("a + 10 L");

	SECTION("Space between a unary minus and a number") {
		requireIncorrectExpressionDetection("52 a - 53", ops);
		requireIncorrectExpressionDetection("- 52 a 53", ops);
	}
	SECTION("Minus as an operation") {
		requireIncorrectExpressionDetection("52 - 53", ops);
	}
	SECTION("An operation that is not present in the input file") {
		requireIncorrectExpressionDetection("1 a 2 b 3", ops);
		requireIncorrectExpressionDetection("6 b 2 a 5", ops);
		requireIncorrectExpressionDetection("1 b 1 b 1", ops);
	}
	SECTION("Two consecutive operations") {
		requireIncorrectExpressionDetection("1 a a 2", ops);
	}
	SECTION("Two consecutive numbers") {
		requireIncorrectExpressionDetection("1 a 2 2", ops);
		requireIncorrectExpressionDetection("1 1 a 2", ops);
	}
	SECTION("Only numbers") {
		requireIncorrectExpressionDetection("1 2 3", ops);
		requireIncorrectExpressionDetection("5 6 7", ops);
	}
	SECTION("Only operations") {
		requireIncorrectExpressionDetection("a a a", ops);
	}
	SECTION("Incorrect symbol") {
		requireIncorrectExpressionDetection("1 * 2", ops);
	}			
	SECTION("No closing bracket") {
		requireIncorrectExpressionDetection("(1 a (2 a 3)", ops);
		requireIncorrectExpressionDetection("((1 a 2) a 3", ops);
	}
	SECTION("No opening bracket") {
		requireIncorrectExpressionDetection("(1 a 2 a 3))", ops);
	}
	SECTION("Incomplete expression to one side") {
		requireIncorrectExpressionDetection("1 a", ops);
		requireIncorrectExpressionDetection("a 5", ops);
		requireIncorrectExpressionDetection("(2 a 6) a", ops);
		requireIncorrectExpressionDetection("a (2 a 6)", ops);
	}
}

TEST_CASE("Single operand valid expressions are correctly evaluated")
{
	std::stringstream ops("a + 10 L");

	SECTION("Simple addition positive") {
		requireExpressionEvaluatesTo("1 a 2", ops, 3);
		requireExpressionEvaluatesTo("2 a 3", ops, 5);
	}

	SECTION("Simple addition negative") {
		requireExpressionEvaluatesTo("-1 a 2", ops, 1);
		requireExpressionEvaluatesTo("2 a -53", ops, -51);
		requireExpressionEvaluatesTo("-52 a -53", ops, -105);
	}

	SECTION("Multiple numbers") {
		requireExpressionEvaluatesTo("1 a 2 a 3", ops, 5);
		requireExpressionEvaluatesTo("3 a 5 a -2", ops, 6);
		requireExpressionEvaluatesTo("-51 a -2 a 8 a 20", ops, -25);
	}

	SECTION("Brackets") {
		requireExpressionEvaluatesTo("1 a (2 a 3)", ops, 6);
		requireExpressionEvaluatesTo("(3 a 6) a -2", ops, 7);
		requireExpressionEvaluatesTo("-51 a (-2 a 8)", ops, -45);
	}
}

TEST_CASE("Two operand valid expressions are correctly evaluated")
{
	std::stringstream ops(
		"a + 10 L \
		 b - 10 L");

	SECTION("Simple positive") {
		requireExpressionEvaluatesTo("1 a 2", ops, 3);
		requireExpressionEvaluatesTo("2 b 3", ops, 1);
	}

	SECTION("Simple negative") {
		requireExpressionEvaluatesTo("-1 a 2", ops, 1);
		requireExpressionEvaluatesTo("2 b -53", ops, 55);
		requireExpressionEvaluatesTo("-52 b -53", ops, 1);
	}

	SECTION("Multiple numbers") {
		requireExpressionEvaluatesTo("1 a 2 b 3", ops, 0);
		requireExpressionEvaluatesTo("3 b 5 a -2", ops, -4);
		requireExpressionEvaluatesTo("51 a -1 b 8 b 20", ops, 22);
	}

	SECTION("Brackets") {
		requireExpressionEvaluatesTo("1 a (2 b 3)", ops, 0);
		requireExpressionEvaluatesTo("(3 a 6) b -2", ops, 11);
		requireExpressionEvaluatesTo("-51 b (-2 b 8)", ops, -41);
	}
}

TEST_CASE("Many operands, different priority valid expressions are correctly evaluated")
{
	std::stringstream ops(
		"a + 10 L \
		 m * 10 L");

	SECTION("Simple") {
		requireExpressionEvaluatesTo("1 m 2", ops, 2);
		requireExpressionEvaluatesTo("2 a 3", ops, 5);
		requireExpressionEvaluatesTo("20 m 3", ops, 60);
	}

	SECTION("Multiple numbers") {
		requireExpressionEvaluatesTo("1 a -2 m 3", ops, -5);
		requireExpressionEvaluatesTo("3 m 5 a -2", ops, 13);
		requireExpressionEvaluatesTo("51 a -1 m 8 m 20", ops, -109);
		requireExpressionEvaluatesTo("-50 m -1 a 3 m 20", ops, 10);
	}

	SECTION("Brackets") {
		requireExpressionEvaluatesTo("3 m (5 a -2)", ops, -30);
		requireExpressionEvaluatesTo("(51 a -1 m 8) m 2", ops, 86);
		requireExpressionEvaluatesTo("-50 m (-1 a 3) m 2", ops, 300);
	}
}

TEST_CASE("Complex cases")
{
	std::stringstream ops(
	   "a * 23 1 \
		b / 5 1 \
		c * 26 1 \
		d - 36 0 \
		e / 27 1 \
		f + 40 1 \
		g - 27 1 \
		h / 4 1 \
		i + 27 1 \
		j / 21 1 \
		k - 30 0 \
		l / 7 1 \
		m / 14 0 \
		n * 5 1 \
		o - 1 0 \
		p * 6 1 \
		q * 23 0 \
		r * 21 0 \
		s + 27 0 \
		t * 35 1 \
		u * 2 1 \
		v * 33 0 \
		w - 13 1 \
		x * 26 0 \
		y / 40 1 \
		z + 10 0");

	SECTION("Complex expression") {
		requireIncorrectExpressionDetection("2 v -1635 m -4748 n -4579 ) s -1018 h -1028 i 3102 h -4097 p -3837 o -151 i ( 783 ) x 3684 p 3649 u ( -693 s ( -4397 m -2902 l ( -3260 x 4690 d 115 x 2069 s -4872 ) u ( -732 i -3342 ) w ( 3895 b 3598 v -928 n 2080 o ( -3508 ) d -3374 ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("4742 t -2847 p ( 623 u -3524 s 2749 v 2082 a -3255 ) k ( -1074 v 1766 g -1177 y -4996 m -3818 r -4029 t ( 2314 y -704 ) v ( 3611 ) m 385 r -1285 c ( 1151 ) e ( 4685 b -647 t -1833 u ( 1355 e 2794 g -1279 ) h ( -346 i -875 f -2037 m ( 3425 y 4402 h -759 ) ) ) )", ops, 1.5659433143311414e+20);
		requireExpressionEvaluatesTo("-3599 e ( -4453 u 4245 k 1308 d ( -3023 l -4060 ) j -792 i ( 2059 g ( 3075 b 4170 u ( 236 v 1381 z -353 o 4961 j ( 166 h ( -4394 ) x ( 1306 c ( -1952 v -746 z 2735 n ( 644 ) m ( -3965 i ( -231 s -3861 x ( -1424 a -3623 k 765 a ( 589 z ( -1575 f ( -4292 g 2176 h ( -2333 e -4596 ) l 4061 ) d ( -972 r ( -4484 p 3774 a 4052 c -3722 u 1241 j ( -2279 p ( 394 h 4245 u 1603 ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) )", ops, -65.6993606761429);
		requireIncorrectExpressionDetection("-3032 h ( -2334 i ( 4549 n -3277 y ( -4846 ) l -251 z ( 1856 m -2894 h ( 1338 ) v 968 q -2413 u ( -2363 u 406 x -3625 ) t ( 4281 q ( -3671 x ( 1546 i 3228 s -2696 ) j ( 1551 f ( -1112 ) a -10 z ( 3152 p 3576 w ( 3207 k 95 m 4946 ) a 2361 ) ) ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("-2426 f ( 3663 z 2778 o ( -1964 i ( -4507 t -4171 t ( -232 p 905 ) i 3230 i -4193 z -4880 ) q ( -2603 o -4995 ) i 135 p ( 1097 c ( -871 ) e -4809 i -4094 ) x 1075 k -254 ) t ( 1065 v ( 2516 p ( 4511 x 731 a 1730 k ( -2726 m 199 w ( -4154 k ( -511 k -1353 ) r ( -4760 t ( -69 ) r -670 ) x -1986 ) l 4259 ) n ( -4526 ) c ( -2289 g -4066 ) r -1730 ) ) ) )", ops, 3.7519167693464676e+60);
		requireExpressionEvaluatesTo("-1934 z 328 h -3665 e 958 u -902 m 2358 g 3145 u 4795 f ( -1970 b -2037 s 1844 ) d ( 2532 i ( 2070 t -2134 ) p -1400 ) v -955 o ( 2212 p ( 228 y 1654 r ( -4443 ) ) )", ops, 2839982747557677.0);
		requireIncorrectExpressionDetection("-3530 m ( -3606 k -3303 ) g -3907 g ( 4902 ) z ( -3110 ) c ( 3170 ) x -4687 i ( 4839 b 4155 l 4651 w 4902 q ( -808 p ( -86 x ( -361 c 4007 ) ) ) )", ops);
		requireExpressionEvaluatesTo("4457 d 4439 d ( -2771 e ( 3761 f -534 v 92 c 35 s ( -3054 n -7 ) r ( 2465 ) d 1736 ) a ( -4512 r ( 1471 ) r ( 361 q ( 3776 h -3270 h 4770 u 2853 l 454 r -212 d -464 v ( -2836 n 3101 u ( -173 y -1375 w ( -1780 m 1866 o -4443 ) i ( 1230 s 924 ) j -2730 k -2693 d 1712 e ( -2651 u 3977 a -4064 p 3823 ) b 3189 c -2261 n -3957 p 3591 ) ) ) ) ) )", ops, 18.0);
		requireExpressionEvaluatesTo("302 q -3056 c ( -4263 y ( -4359 e -4496 w -1376 h ( 60 l -4530 ) a 1185 ) q 353 t -1819 f -4418 c 1917 r -3060 n ( -4500 u -1816 b -2336 q ( 3453 u ( -3046 s ( 4685 ) o 4116 v ( -2753 h 539 w ( 925 n -2446 h -1978 x ( 140 a 2845 ) o -429 u ( -956 ) a ( 3828 ) e ( 662 k 3560 m ( -4625 g 2946 u 4731 ) t 2993 m 1173 ) ) ) ) ) ) )", ops, 3.5801124752000896e+17);
		requireExpressionEvaluatesTo("-2497 o ( -3245 l -4461 ) y 4226 e ( -1338 ) z -35 h -3504 w ( -4432 h 2041 a ( 3817 ) p ( -2143 z -2588 h 1704 ) z ( -1314 k ( 2738 g 1968 r 120 e 4889 x ( -4233 j ( -4894 u ( 554 i 4381 c -1319 c ( -1530 ) m 3947 ) ) ) ) ) )", ops, -2497.00998858451);
		requireIncorrectExpressionDetection("936 i -3094 n ( 1638 ) r ( 3337 i -1561 ) m ( -4600 p ( -4986 h 1858 m ( 1902 w ( 1202 ) z 3678 b 1999 i -4945 p -450 ) y ( 724 y ( 1165 m -1648 i 669 y ( -776 q 4828 f -2339 o ( 3167 g 4931 ) c ( -2806 s ( -120 ) e -749 ) i ( -3692 ) z 2069 ) w ( -4993 a ( -4910 ) ) ) ) ) )", ops);
		requireIncorrectExpressionDetection("-1817 u ( 1081 ) n -3489 z -2252 o -65 t ( 1980 a ( -1330 p -646 v 2410 i ( -4660 ) o ( 3850 a ( -783 l ( 2122 r -3326 j -1772 ) v ( 4086 b 1353 ) s ( -2215 ) z -1695 k ( -4810 ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("-989 n ( 4599 m -1638 ) v ( -3610 k -196 ) p ( -3898 c -3082 ) v -1413 y -1106 v 4934 m -875 k ( -2173 l ( -2871 x -1238 j ( -3189 c 3294 p ( -2533 h -1755 ) w 4008 s -4419 m -62 ) q 3373 e -955 v 3397 b ( -2800 ) ) )", ops, -7472020485696.734);
		requireIncorrectExpressionDetection("-454 b ( -1525 n ( -2102 z -3941 e 939 s 4710 s ( 3074 ) m ( 1929 a -745 ) q -1081 f -4796 m ( -3312 w 4136 x -529 g 3517 x -2922 a -729 u ( -3247 m ( 59 j -1922 j -4462 ) w 3438 f -283 d ( -3807 j -1434 q ( -4078 ) f 1038 g -3566 c -2902 c -2998 h -2885 a -1602 ) ) ) ) )", ops);
		requireIncorrectExpressionDetection("3694 x -4295 l ( 4328 j 2828 g 1571 ) b 3462 q 2866 z ( -1858 k -394 k ( -2335 y 3641 i ( -3434 r 3972 i 2245 d ( -640 v -1657 e ( 3873 w 633 l 45 ) w ( 2626 f ( 475 ) g -49 d ( -4480 k ( -171 i ( -3688 c 2684 x ( 2573 ) ) ) ) ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("-4421 h -4412 o -2986 r ( -4862 h 424 g ( 2002 n -4805 z -70 f ( -2322 ) g 4003 b 677 f ( -4748 k ( -1712 c 1651 ) w ( -1897 h ( 974 h 3022 ) r ( 2833 y -3199 ) i -4902 x -980 i ( 3013 o ( -916 ) ) ) ) ) )", ops, -33609.65337078997);
		requireIncorrectExpressionDetection("4370 u ( -1920 m -1278 ) f 4495 x 3477 y 3177 j 1519 d ( -4525 ) e -2698 v -367 t ( -4419 u ( 2354 e -2568 p ( -1078 q ( -4556 b ( 467 i 1735 ) g 3319 ) x 3094 o -1608 e 403 i 3844 ) s ( 2023 ) e ( 2008 i -1082 y -2232 s ( -3780 c 132 q ( 4234 ) ) ) ) )", ops);
		requireIncorrectExpressionDetection("-4400 i 2108 z -1166 k -2212 x 2643 k -4616 p ( -450 q ( -1646 g 3495 y 1102 r ( 4480 a ( -2760 u ( -1561 ) d ( -3557 f ( 3278 m -2475 v -4816 t -4588 m ( 3415 ) p -2681 u -1444 f -2910 c -3044 x 2186 n 3028 ) n 1122 u ( -1994 ) r -2371 ) v ( 3921 b ( -4456 ) a -4763 ) q 335 ) j 4609 t 2653 ) ) )", ops);
		requireExpressionEvaluatesTo("4590 f ( 3355 r -2838 ) n 782 w 2226 p 414 r -3472 f -2625 i 1600 a ( -563 d 2724 t ( 3686 g ( 3275 e ( 2828 e ( -2698 f ( 1633 m ( -2254 e 4199 e 2262 z ( -2015 ) t ( 2180 w ( -4294 u 1912 f -3849 v -1771 l ( -2485 i ( -639 y ( -922 ) g ( -302 w -663 h 1196 u -1386 a ( -4544 p 799 g 1689 w 3830 u 3959 q 1009 ) k ( 1751 ) n -3744 d ( 2153 ) ) ) ) ) ) ) ) ) ) ) ) )", ops, 5.7274537382215336e+23);
		requireExpressionEvaluatesTo("-4509 m ( 2845 k ( -3199 f ( 220 k ( 4655 y ( -1725 n -4705 e 3472 c ( 2621 u ( -4497 j ( 2252 n ( 2776 z ( -1974 t ( 4331 q ( 3376 ) u 4701 h ( 3807 ) i -2382 w ( 2049 ) r 3262 b ( 3862 d -4108 ) d ( -2843 s -3014 ) t 2296 e ( 3015 ) t -527 k -1254 ) l 795 ) c 1836 m -4486 y -4743 ) i ( 2552 k -4123 j 4876 k ( -1834 p ( 4440 e -3871 l ( 2684 s 2018 o 4257 ) a ( 1284 a -1631 w 2192 ) ) ) ) ) ) ) ) ) ) ) )", ops, -0.011772237251169);
		requireExpressionEvaluatesTo("1255 s -3170 m 3532 y ( -107 ) j -145 h ( 4877 ) j ( 2423 v ( -3721 b ( -450 o -4754 x ( -3967 ) m -3880 z 2289 ) m ( -1980 ) e 2740 a ( -4332 n 2753 ) d 46 w 4634 ) f ( -1599 e 3745 l ( 4454 t -1251 c -4787 j -3305 q ( -3022 ) s 2560 ) ) )", ops, -3355.760561136443);
		requireExpressionEvaluatesTo("-996 t ( -2733 b 1166 q -689 ) m ( -93 o ( -2157 n ( 767 d ( -362 z 3869 ) i ( 3697 ) e 182 ) m ( 3410 f -1630 ) f ( -1508 w ( -2089 r ( 2478 ) r ( 2348 o -1368 k ( 2258 s ( 1167 b -3498 ) ) ) ) ) ) )", ops, 0.036433278277033);
		requireExpressionEvaluatesTo("-3412 i ( -2898 h ( 376 ) g 1446 ) n -844 h -1030 n ( -2344 p 1838 d 1508 ) y -1981 a -897 a -1776 d -3588 v 3657 p 3130 t -3362 v ( 364 y 4504 ) f 4115 z -4607 g ( 4235 g -2325 q -43 g -1346 e -4662 t -1597 e 212 h ( -963 y ( -4246 y -4008 ) q ( 4262 ) v -4528 p 3328 ) )", ops, -0.0);
		requireIncorrectExpressionDetection("874 h 4308 o -4949 e ( -3133 ) h -1526 r ( 4941 j 1474 r 979 i 1224 k 476 w ( -3962 i ( -4916 ) x ( -2870 h ( -1945 b 1657 d 1137 l -3181 q ( 2935 i 4499 y ( 529 ) q -2406 ) l -2659 a 2412 ) f ( -4154 r ( -299 g -2010 o ( 2074 ) d ( 1153 ) w ( 1815 p ( 4740 u ( 1414 s -2270 x ( -3877 z 758 ) i ( 4107 r ( -173 c -1045 b -4163 ) ) ) ) ) ) ) ) ) )", ops);
		requireIncorrectExpressionDetection("-142 q -3186 t 4123 g ( 3119 ) a 2943 c ( 4653 i 1245 j -1766 ) a -803 k ( 2181 i -413 w ( 3873 x ( 1328 p 3511 a -4040 a ( 940 ) o 642 i 3302 r ( 1955 a 3809 a 4048 q 623 s 4775 ) d -2325 p ( -1618 ) d ( -1544 k ( -300 b -3845 u 3070 ) c -4950 ) a -3139 n 1244 o ( 3590 z 2975 k ( -4954 l 4977 q ( -4369 y 2659 h ( 3488 x 3029 x -2547 ) x ( -1630 z -2888 r -278 ) ) ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("-3204 y 4353 j 2042 j -47 k -1864 n ( 3780 m -4167 l 3615 o ( 2207 ) a 3375 e ( 728 ) n ( -1002 ) h ( 72 ) j -3382 v ( 3566 q ( -4340 s ( 1768 i 1105 g 4743 c 1237 ) ) ) )", ops, -789502370139.6301);
		requireExpressionEvaluatesTo("-633 k -3555 b -2847 s ( 975 ) b 3131 n -2894 e 1201 y ( 934 f -3016 ) z -1904 d 4150 n ( -1547 w 3424 i 4795 ) l ( 891 )", ops, -5.666919970006986);
		requireIncorrectExpressionDetection("-3235 q 3139 c 1627 b -3686 w ( -1935 d -1516 v ( 933 k ( 2022 v -4593 ) k -3575 b ( 3337 o -750 ) n 267 s 1231 ) x ( 3254 g -1247 ) t -3596 s ( 1176 ) n -933 b -2588 g -253 ) m ( 3076 ) y 476 e 1187 f ( 1460 f ( -1782 u -235 ) k 2027 ) i -2925 s 2761 i 4304 a -3383 d ( -1708 a -869 a ( 471 ) )", ops);
		requireIncorrectExpressionDetection("-1628 n 2180 l 2543 n -2265 z ( 4351 c -526 ) s -2516 u 711 c 4697 s ( -2285 ) i 825 l -1955 e ( -1162 w 1983 ) x -2189 d ( 1079 v -4453 c ( 2780 ) r -4421 j 3041 ) n -2398 w -1213 h -2604 k -4457 q -4670 a ( -4190 c ( 3234 ) )", ops);
		requireIncorrectExpressionDetection("-502 x 4330 i ( -4288 o 3974 s -2111 ) e ( -4659 ) o -803 q ( 1436 y 2711 k -1767 k 2897 u 3453 d -4 y ( -3609 f ( 1443 t ( 998 f -4271 ) k 4487 v ( -3856 y -3637 h -4150 l -551 j 3721 r 2345 ) a -395 o ( 4542 j ( 1014 o 2644 y ( 982 ) ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("4316 q 3315 k ( 463 v -62 u ( 3067 z ( 1396 c -960 h -3212 w ( 2528 ) p 1025 ) e ( -4415 l ( -4984 j ( 1030 ) q ( 860 g ( -2448 r -535 ) ) ) ) ) )", ops, 380000566971.9764);
		requireExpressionEvaluatesTo("-3073 l -4561 y ( -3922 i -2522 ) r 4867 t ( 1261 r ( -637 ) n ( 1575 d ( 73 h ( 2991 q ( 2314 c ( -451 ) ) ) ) ) )", ops, 7.05118e-10);
		requireExpressionEvaluatesTo("3035 a 3931 g -393 t ( -2060 a -862 o -4972 u -2931 w -3324 i -4265 f -1490 ) m -1902 v ( -2980 g ( -4789 g -1498 m -4646 ) i ( -3212 t -744 c -4120 t 3173 p -2981 d ( -513 y -4742 m ( -4820 a -3642 u ( -3225 k 4333 ) t -2700 t 3753 v -4740 g ( -645 f -1544 c 593 k ( 3073 x ( 3411 q -546 ) ) ) ) ) ) )", ops, -2.17796635e-07);
		requireExpressionEvaluatesTo("-4231 k ( 4360 o ( 927 u 45 c -325 o ( -637 h -73 e 3081 m 1803 ) h -4430 ) h -1544 ) q ( -3642 v 3595 c 3472 v 4161 r 3623 )", ops, -1.2513445867651475e+20);
		requireExpressionEvaluatesTo("-2199 z 4981 y -1247 e 2874 k -3658 a ( -2194 ) v ( 4701 j ( 3411 ) p -2084 n -980 t 3741 p ( -573 ) p -2776 ) s ( 45 s ( -3779 ) u ( -2018 x ( 1616 a -2076 ) e ( 1277 ) ) )", ops, 2.2471633839652804e+16);
		requireIncorrectExpressionDetection("246 q ( -630 x ( 2900 r 3328 ) s -2840 c 569 ) j ( 4259 ) i -1398 k -3444 l ( -3056 w 1709 ) g -2963 t ( 1273 h 2857 ) a ( -3068 j ( -3522 y 2060 z -3257 o ( -4517 j 1656 ) l ( 4426 ) l 3639 u ( 2097 f -746 i -2535 ) i ( 1560 ) l ( 1076 ) v ( -2963 j ( 1159 ) y 2009 ) ) )", ops);
		requireExpressionEvaluatesTo("-2407 a ( 3370 q -2340 ) u -230 u -1754 a 1362 f 2452 v ( 2389 s -4057 ) w 4745 u ( 709 u 860 p 2948 i -140 t 4723 a ( -3398 q -4800 c -619 q ( -4320 x -1812 l -3216 q 2099 n ( 1273 z -2780 m -3203 p ( -2686 s 3839 ) ) ) ) )", ops, 3.3622370458889084e+50);
		requireExpressionEvaluatesTo("-4156 x -319 v ( 3917 n ( -2071 q -1569 b 2303 s -3994 k -568 j ( 1598 e 1976 t ( -1809 ) ) ) )", ops, 6717293858.062408);
		requireExpressionEvaluatesTo("-3546 o ( -3739 ) c ( -1929 ) h ( -206 n -130 f ( 3726 ) j -4917 i -4123 ) u ( -3457 r -4855 g -1042 u -4330 x ( 4094 i ( 3662 e ( 2314 v -2184 ) z -2845 r ( -74 a -4852 m -2769 f 3430 a -1594 d ( -1979 d ( -4104 t -3800 ) t ( -1031 ) i 156 ) p -2747 ) ) ) )", ops, 2.056567921346166e+19);
		requireExpressionEvaluatesTo("1761 i -3232 r -3704 s 1430 u 4068 o -2297 p ( 4871 x 2513 s ( 1063 ) z -4876 r ( -3727 ) g ( -3102 ) w 2344 d -1534 d ( 4681 q 2511 k -1858 z -2510 n ( 1222 ) n ( 4956 ) g -2683 ) e ( 2545 ) a 4458 k -1216 n ( -1417 e -2322 z -2654 ) t 4345 j 596 s ( -2116 x 1107 g 3047 u ( 87 ) y ( -1043 y ( -1402 a -885 k -4943 ) r -3323 x 211 ) ) )", ops, 4.0571593042623693e+18);
		requireExpressionEvaluatesTo("-4163 s 2210 l -4393 e -3190 v 3558 i ( -3755 g ( -2292 ) r 1770 x -2374 i -1754 q 800 ) k 2679 d 4432 i ( -2927 s 1720 h ( -1682 ) )", ops, -2.28378e-10);
		requireIncorrectExpressionDetection("2394 t ( 1727 o -765 ) o 1338 m -214 k ( -2715 u ( 2498 ) o -2278 q ( 4506 p 4046 ) a ( 1520 y -4150 ) i 4083 ) b 1914 v ( 2980 ) c ( 4405 v 2820 o 3190 ) h 862 i ( 864 z 2395 u ( 3505 ) s -292 ) j ( -2937 s 4915 m ( -4835 r ( -3617 q 2397 ) ) )", ops);
		requireExpressionEvaluatesTo("3799 k 3164 x ( -2303 y ( -3169 v ( -1784 p ( -1675 f -1522 e 1718 e -4626 ) f 1249 ) x -1910 f ( -4940 c ( 1260 a ( -889 n -3953 ) a 4456 x ( -1200 n -3402 e ( 4728 ) ) ) ) ) )", ops, 0.0);
		requireExpressionEvaluatesTo("2041 e 224 b 3987 f -4037 j 2981 v 1921 y ( 2286 ) u -829 k ( -858 q 1872 n -4514 j -4669 ) e 2720 f -4375 l 266 v -2418 v 1680 z -3996 z -1593 i -4598 n ( -3337 p ( -892 ) n -1563 z ( 1461 ) h -1512 o 1535 d ( -996 ) )", ops, -78.55038404856496);
		requireIncorrectExpressionDetection("-398 l ( -254 w -1522 ) v -3352 o ( -3286 r -358 j ( -530 ) i ( -70 ) g 3746 k 3749 l -2081 q ( -252 w -3495 ) u -447 e -331 e -4008 l ( 3791 o ( 943 k ( 3958 s ( 243 ) x -1012 ) z 2014 n 4211 b -578 v -607 ) z -37 ) t ( 4104 c 475 f ( 2413 d ( 1388 g ( -1059 ) g ( -996 l -152 l 1584 s ( -1324 g -552 s -4585 k 1281 ) e 3093 d ( -4569 v -92 t 2615 z -4483 v ( -1933 u 1302 q 4400 b 3826 ) ) ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("4690 b 2024 s ( -4796 s 943 q ( -2754 ) w ( 2306 c ( 1149 ) y 1358 g ( -4176 f ( -3895 p 999 ) w 534 ) l 4887 l ( -3284 z 3599 m ( -2265 i 217 p -705 m 1791 o ( -2558 l ( -2171 ) y 421 g 219 f -1662 ) h ( -2848 p ( -1465 p ( -2033 p -483 g ( 3803 g ( -3924 l -2685 ) a 4304 ) ) ) ) ) ) ) )", ops, 0.000441879781505);
		requireExpressionEvaluatesTo("1265 f ( 703 ) r ( 2060 n ( 2101 j 3386 ) w ( 1629 ) k 2408 n 4192 c 1282 q ( 1084 ) q 2544 s ( -1723 l -4883 r ( -2697 m ( 3224 s 2249 x 1053 ) y -1348 i ( 3831 k ( -1866 h ( 4527 ) q 1612 z ( -2932 r ( 4632 x -4834 n ( -4443 ) c 329 ) h ( 3425 ) ) ) ) ) ) )", ops, 4.6842619531203315e+22);
		requireExpressionEvaluatesTo("2626 d 1558 z ( -2142 m ( 4584 c -333 r ( 4881 q ( 2089 ) d ( 2289 x 2988 w 1914 m 1568 ) w ( 4497 k ( -4954 a ( 543 e ( 602 c -1900 p ( 4134 g ( 1246 t 2838 h ( -4283 g 304 ) q -4082 ) ) ) ) ) ) ) ) )", ops, 1068.0);
		requireIncorrectExpressionDetection("585 v 3545 b -4499 m -4163 d -4770 k -1165 k -786 t 4908 b -1876 e 1073 p ( -3310 m 2898 ) f ( -4726 ) j 3414 g ( -1659 ) h 618 u 545 s -3639 y 2556 y 4353 i ( -4403 h 708 v 1735 i ( -4865 r ( 2515 ) o ( 3899 j -60 n -1998 r -4407 ) v 243 ) )", ops);
		requireIncorrectExpressionDetection("3761 i ( -3984 ) w 1588 t -51 b ( 3855 i ( -81 ) z ( 672 ) w -2794 v -1513 ) h ( -1148 ) a ( 3808 c 3526 v 480 x 4205 ) o ( -3310 y ( 4868 n 3050 u 7 ) w ( 389 b -4657 q -2911 ) o ( -4238 p 2549 ) e -4310 t ( -3568 f ( 3232 c 4740 i 2936 j 2750 q -4285 i -487 i -1685 ) b ( -3740 ) ) )", ops);
		requireExpressionEvaluatesTo("4260 f -357 f -4479 a ( -3454 v -3187 f -1750 t 3650 ) t ( 3661 s -3328 ) v ( -3559 d 1728 m ( -1371 t ( -2085 e -1525 j -2402 g -3590 o -1728 o ( -4504 n ( -2425 q 3047 ) x ( -3501 b 4636 b ( -3593 w ( -2128 ) g 1535 a ( -845 h 999 s ( 728 c ( 3969 e 3504 ) y 4951 ) d 1462 ) ) ) ) ) ) )", ops, 5669954240.046191);
		requireIncorrectExpressionDetection("1675 t -1519 e 4877 a 365 r 1811 a ( -4918 ) x -156 e -613 b 2148 j 1388 l ( 3965 q 1488 q ( 892 v ( 3385 k -2026 s -3115 ) b ( -2964 x ( -1647 s 603 ) x ( -3842 h -751 u ( -2663 w 4960 s ( 2861 ) w ( 324 p ( 4808 h 2284 ) f -3128 a ( -154 v -4611 ) q -4049 w 789 e ( -624 ) p ( -927 ) w -2465 ) ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("4569 z 1256 d ( -2383 m 1902 o ( -3093 u ( -400 f 3884 j ( -813 q ( -4962 q 129 g 1804 c 938 m 3071 ) q ( -2970 ) i -4275 e -4088 p ( 1249 a ( 32 y -4664 f 3692 e ( 1457 y -286 k ( 2255 i ( 1823 ) a ( 3131 q -529 c ( 933 e -3687 ) n ( 79 f -3322 ) o ( -3560 o -1123 a -1403 x 1007 ) w ( -2075 ) ) ) ) ) ) ) ) ) )", ops, 5233.881553407323);
		requireExpressionEvaluatesTo("-2872 m ( 3800 f -3494 u 963 x 3445 ) s 4573 a 2661 h -3664 f -2596 d ( -4128 d 3573 h ( 2049 h -2478 ) u 1523 e -4444 z -527 i ( -1972 u 1017 n 1713 ) y ( -4911 ) y -1775 w 3642 n 393 ) j -430 y ( 1634 i 737 p ( 3770 m 4605 o ( -2371 t 185 p -1520 b ( -60 ) q ( 2572 ) w -3570 i 924 ) j 1223 ) d -3334 e 933 ) z -2163 y ( -673 z ( -4382 a ( -3023 ) u 122 k 4287 ) w ( -774 s -2822 ) t 1233 d -3143 ) x ( -445 v ( 2249 ) )", ops, 0.0);
		requireIncorrectExpressionDetection("4941 j 2945 e -4183 d -2525 a 1647 d -855 i -3985 o 3096 p -4740 h 1628 r 4666 h ( 309 n 438 o 2541 ) b 1809 q ( -3777 ) j 3969 o ( 2157 p 3375 g 4727 s 568 z -3290 ) n ( 471 j ( -30 ) )", ops);
		requireExpressionEvaluatesTo("-3801 s -2489 t ( -3246 k ( -1482 y 2046 y -2330 x ( -3892 ) j -3840 w -226 u ( 4913 n 4365 u ( -2793 y ( -2776 x ( -1534 ) x 1546 ) n 1744 a ( 2812 ) e ( -2675 a -3118 k ( 4107 j ( -3641 v ( -2963 l ( 715 ) t ( 153 ) z ( -1629 j ( 3870 m -1438 w ( -2666 e -3225 r ( 776 n ( 2374 o 2431 l 2532 a 1169 s 4382 u ( 2018 q 1731 n 449 r -3676 r 4929 l -2140 s 2265 ) k -2332 ) w -3289 y ( -306 v ( 3706 ) y -3404 ) ) ) ) ) ) ) ) ) ) ) ) )", ops, 5106007.5183035135);
		requireIncorrectExpressionDetection("-2482 m -3726 g 444 t -1420 s ( -1104 j -812 k 4015 ) b -2796 a 575 p -848 e -4628 t -736 v 2390 l ( 1789 b 3304 u ( -180 f -335 u 1444 ) x ( 1058 p -3292 p -2317 u 4890 u 1261 o -2973 v -1513 z 2367 ) t -195 f 1804 z ( 2977 ) f 357 y 1473 a 161 e 1352 )", ops);
		requireIncorrectExpressionDetection("-4838 c 734 e ( -4561 g ( -4188 u -3027 m -4064 s -2917 v -1303 ) g ( -4074 ) z -2768 l -3814 t ( 1983 m ( -2281 ) n -500 d -4909 q 375 ) s ( 1259 k 335 n 2938 ) y 4782 o ( -2734 p 4041 ) c ( 1984 j -3946 l -339 v -463 o ( 4004 b -2285 ) z ( -1944 o ( 2762 ) v 2847 a -4382 o -3443 ) z ( 2141 j ( 3739 g 2932 q 1448 e ( -2337 ) b ( -4990 ) g ( 2132 z 1931 x 4797 r ( -156 n -1179 ) e 3418 d 1249 j 528 ) ) ) ) )", ops);
		requireIncorrectExpressionDetection("3686 f ( 3359 i ( -3589 k 2459 ) f ( -1772 l 224 s 3265 f 1748 d -3297 q -2031 g ( 3420 y ( -4755 c ( 3012 ) j -1354 k 4997 o ( 4180 o 3449 m -4741 z 3651 w ( 1682 b -1081 c -88 i ( 4982 y 4436 ) v -2601 e ( 3017 l -2165 c -2653 w 1890 o ( -3396 u 4126 ) y ( -3205 w 3256 e -3511 s 4539 u 3950 r -1339 t ( -4709 ) v ( 4174 g 785 ) z ( 3882 ) w ( 3600 ) d ( 2727 ) n 3243 ) ) ) ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("474 r 456 h -3680 s ( -1832 n -151 b ( -4625 k ( -2150 x -2764 j -11 ) k -2031 a -4271 y 4004 o ( -2783 f 1459 t -1668 x -1689 w ( 4142 w ( 1720 c 1862 e ( -415 s ( -1223 a -4131 ) b ( -2745 u -3206 r -3596 m ( 3235 f ( -2725 ) ) ) ) ) ) ) ) )", ops, -58.73478378020468);
		requireIncorrectExpressionDetection("3821 s -1427 v 3695 o -3289 a ( -2399 v 4975 r ( -1528 ) j -4189 o ( -4218 w -266 ) w -1324 q 764 x ( 4358 ) a ( -202 r ( 4644 y -2354 m -1100 j 3138 ) w 1472 p -2999 ) r 908 v ( -2047 g 2287 n ( -2984 ) t ( 3342 n 3683 e -4299 t -3206 q -2858 ) ) )", ops);
		requireIncorrectExpressionDetection("2916 s ( 2616 ) j 4682 n -3433 x ( 4146 e ( -3203 ) z 1124 z ( 406 p ( 3065 h ( 2286 c ( -2002 h 2875 g ( 4845 r ( 2560 o 2939 g -505 s ( -1459 z 4227 z ( -1716 u ( -2272 r ( 229 k -2764 ) u ( -2274 v -1977 g ( 3502 t 4919 w 319 f ( 4958 l ( 3304 o 3964 ) ) ) ) ) ) ) ) ) ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("-3705 y ( -1886 b 4320 z -2253 n -1608 v 2588 ) z -1096 u ( 183 j ( -3736 ) b -1887 o ( -4310 m ( 4308 l ( 1219 b ( -3302 n ( 2328 g 3017 t 3269 z -721 s ( 3508 ) ) ) ) ) ) )", ops, -0.02849113021503);
		requireExpressionEvaluatesTo("-4613 x -1400 q 3935 o 2015 s 3037 k -1855 u -4576 u ( -4608 v -3099 ) c -362 y ( 387 h 3955 e 4019 o -4228 m -1364 ) a 4730 w -1886 l ( 3176 l ( 4574 ) n -2144 q -2331 )", ops, -545384440004.43677);
		requireIncorrectExpressionDetection("-3777 i 3154 j ( -3457 p 2710 r ( -3408 b 2071 a -3362 g -1984 f ( -93 ) s ( 3847 a ( 151 j -1063 r 4586 h ( -2670 i -3585 y ( 2384 z 3751 ) v ( 3026 ) f 3771 h ( -2807 e -1664 ) l 2844 c ( -4900 w 2976 q 2567 k -1523 s ( -3110 m ( 661 d ( -4864 ) r -1733 a -2677 g ( -4916 e ( 4480 j ( -2924 ) p ( 2486 b 1822 z ( -1239 g ( -988 r -3444 o ( 649 ) t 1716 ) g -4765 y -1924 ) s -3346 d ( 3399 ) s ( 1500 ) ) ) ) ) ) ) ) ) ) ) )", ops);
		requireIncorrectExpressionDetection("-3384 l 1544 u 4647 n -4649 z -3304 j 1040 m 2952 p 2365 m -898 i -1050 x ( 155 c -4384 f 654 ) w ( -4026 i -1998 p ( 3237 v -4247 f ( -687 a -1716 ) q ( -1285 ) j 3074 r ( 3668 ) n ( -626 s -336 ) g -833 j ( 4092 j -3942 w -4382 i -2020 o ( 2647 ) x 1195 ) s ( 1851 b -2414 ) ) )", ops);
		requireIncorrectExpressionDetection("-1667 a -1946 r -2880 x ( 2717 x ( 225 s 4023 ) n 3114 r 3390 ) d ( -1611 c 1133 j 1513 ) c ( -3075 d 2488 ) n ( -4568 p 4293 h -2357 p -658 ) c -3165 p ( 211 ) q ( 1025 ) w ( -816 ) q ( 1082 l 3157 )", ops);
		requireExpressionEvaluatesTo("1024 s ( -4413 m 1359 q ( -4256 ) e ( 2466 o 1111 ) n 3 y -3118 ) l 933 d 3171 j ( 1807 j 2659 z -425 q ( -696 ) m -3161 f ( 3877 v 591 ) x 1212 d 4626 ) v ( 4130 n 3242 t ( -1535 w 4707 x -2452 ) d 14 p 2986 x -2382 n ( -2840 ) u 181 l 1856 ) s ( -2895 l 3201 ) h 3126 i ( -4034 ) z 447 a ( 531 y 2487 ) c ( 4561 t ( -1010 a ( 844 ) r -4256 ) d -3800 e ( 397 ) w ( 3251 r -2786 ) )", ops, -23785863695.391975);
		requireExpressionEvaluatesTo("-4517 s 3798 c 3095 r 3511 a ( 1619 q 3971 ) s ( -1163 x -589 a -3428 p 2011 l 4954 ) t -3618 b 2049 n ( 3629 t -2182 v ( 4721 v ( 3212 x ( 4881 z ( 932 g 3403 g ( -2932 p 4257 ) n -4392 r -2121 d ( -3739 ) ) ) ) ) )", ops, -1.400275233106693e+47);
		requireIncorrectExpressionDetection("-593 y -4551 y -2148 y -4330 e ( 243 c ( 1594 t ( 1166 s -4075 w 4135 d ( 2619 ) p 300 p ( -4261 w ( -304 ) z ( 1838 s 2638 y -69 ) j ( -2168 z ( -2534 m 1411 f ( -3215 z 503 y -2335 ) c ( 4841 ) x 209 ) n -1147 l -2619 v -4199 ) k 3744 u ( 479 u 3210 l 1194 j ( -985 k -2824 ) b ( -4104 ) j 1744 d 3296 ) y ( -3005 k 404 ) z 104 t -3314 ) z ( -2591 z -1730 ) w ( 1866 ) ) ) )", ops);
		requireIncorrectExpressionDetection("-4352 a -2596 e 1563 l 2983 q -988 e -3145 s -1841 z ( 3087 ) e ( -2599 w ( -2105 v -2561 o 2398 i ( 1404 m -566 ) j 1187 z 3260 n ( -4599 l -1697 h ( 1063 l -4340 e -68 b -321 v 2635 ) t ( -188 a -4572 y ( -387 ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("-3419 q ( 2679 w 4526 a 4774 ) q ( 3206 ) s -69 y -3607 n -3668 o -4588 p ( -2850 ) p ( -3278 k 4841 ) a 802 y ( 3243 c ( 2138 w -4846 e ( 1400 ) t 4581 r ( -2523 e ( -3866 u 2538 d ( 567 ) v -702 c -3138 r -2394 ) f ( 2110 i 1455 ) n ( 3269 ) y ( -345 ) t -598 n ( -4656 i -1034 f -1709 ) k ( 698 s 4058 y ( 4441 s 4183 f 1846 v -3796 ) u ( 4046 e 4222 ) q -4395 n ( 4960 ) ) ) ) )", ops, -8.686356536641379e+17);
		requireExpressionEvaluatesTo("-1117 u -1727 t -2898 w ( -4537 j ( -4686 t 2286 y ( 1535 z -293 e ( 4098 ) k 2303 a ( 4908 v 1006 c 2745 b -4451 x -2811 w 2914 j -3703 d 3481 s 1177 s -4243 w ( 4561 l 3692 i 1015 u ( 1887 ) x 4246 ) f -631 i 1793 t 3589 j ( -1862 x ( 4599 b 3873 v -1760 n 4084 ) v 4023 r -4783 ) w ( -3828 x 1203 d 1339 u 2367 ) o ( 2270 u -1063 v 2179 a 4668 ) i ( -4999 p 234 v -574 o 3283 ) ) ) ) )", ops, -1900907717097.7048);
		requireIncorrectExpressionDetection("1903 l 2094 j ( 2949 p -4272 l ( -1690 i ( -4282 e -3858 y ( 1584 y ( -2790 y -2351 u ( -495 v ( 1177 q ( -3918 ) g 466 ) g ( 2960 a -3598 ) d 2754 c ( -387 o ( -4284 l 11 ) l ( -3462 u 2091 s 3587 n ( 4049 w ( 4864 x ( -4749 n ( 2267 x -2677 q ( -3217 x -3997 w 552 r ( -286 ) e ( 3289 ) b ( 3762 q -2222 x ( 162 h ( -1456 e -4979 b -149 ) a ( 2835 ) q ( 3158 ) j ( -4084 t ( 1440 n 1232 ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("4378 c -1960 z ( 4064 ) n 1386 w ( -607 ) o -2002 d ( -589 w -4549 ) o ( 2992 ) v ( 2722 h ( -1800 n -3402 k ( -594 ) e -4408 r -3959 p 3843 i ( -1072 ) f ( 2494 f ( -2416 g -4575 ) v 1598 ) t ( -439 n 4333 d 3653 ) ) )", ops, -17093588326.0);
		requireExpressionEvaluatesTo("1215 y -4604 k -1884 m ( -2202 l ( -3408 e ( -454 l -1135 j ( -1860 c 4025 w -4767 e -3481 r -1859 g ( 4008 ) v -3438 ) q 4815 p -1170 q 4091 ) d ( -1938 s ( 4732 ) j 99 ) b -905 w ( -1554 p 2328 o 4336 g ( -737 ) o ( 2935 c ( 4409 q ( -737 g 3884 ) z 2653 s -1628 ) j ( 430 ) d -714 ) k 2970 o ( -3834 ) j ( -244 q ( -4431 ) ) ) ) )", ops, -0.0);
		requireIncorrectExpressionDetection("2533 m 1246 t ( 3324 ) j ( -1951 ) a ( 4137 ) x 4765 u ( -1717 r 4861 c -3580 ) x -2998 o ( -3718 v -1426 s ( -995 i 4594 ) g 3761 w ( -2434 ) c -123 o ( -1225 h ( 2824 z -1841 k -3965 z 556 y 1322 ) w -4338 x -3143 ) j ( 2107 x -2253 y -1587 j ( 3550 ) r -1871 ) p ( 4152 ) b -4113 x ( -1184 s -1392 ) a ( 1170 b -4606 h 2351 f ( -4797 x -1309 b 3407 ) ) )", ops);
		requireExpressionEvaluatesTo("-2107 m 1387 t ( -4365 e 3175 i -3080 g 3635 x ( 346 m 3840 ) w 1706 w 2350 q -4424 m 1623 x 3231 e 1400 x -3966 o -259 j 2003 s 1947 c 536 n -3062 z ( 3202 k ( -999 c ( -2996 ) l -1441 e ( 3424 ) x 3335 p ( -1799 ) o 2992 u ( 1894 q -234 ) ) ) )", ops, 9.206828044e-06);
		requireIncorrectExpressionDetection("-3174 j 678 u 4041 m 1270 j 1694 i -4283 h 1021 p -2995 b -4655 j 1657 d -2370 z -2971 w ( -724 e ( 3186 ) e 144 s 4381 c ( 872 a -2535 u ( 2927 w 945 w 2550 e ( 2892 p ( -4056 ) g -298 g -1048 i 895 ) v ( -3126 w ( 4503 ) p ( -2268 u -840 t 194 ) q ( 4446 u 2852 h -2469 ) g ( -3767 e ( -3666 ) p -4534 ) z 1285 ) f -4237 j -3579 m -325 d ( 670 ) u ( 1264 i ( -1384 e ( 166 e ( 3879 m 2816 l ( -416 o 3316 j ( 3781 ) ) ) ) ) ) ) ) )", ops);
		requireExpressionEvaluatesTo("-4697 i 3778 a -1511 j ( 3524 ) w ( 423 n ( 1357 ) c 2345 u ( 279 ) n -3161 r 4290 x -2390 m 4707 k -2228 )", ops, -1.7550944339774735e+18);
		requireIncorrectExpressionDetection("4928 n 1084 l -3391 j 4311 v -1722 o 3667 v 997 e 3679 m 2649 z -1884 e -4184 r -1915 d ( -3448 n -3400 ) j -4718 m ( 450 v ( -2925 m 4833 x -4126 i ( -906 ) c ( -4091 o 3171 u -3566 f -4455 v ( 625 z 729 r -3095 u ( -1062 o -1405 v ( -1137 ) ) ) ) ) )", ops);
		requireIncorrectExpressionDetection("332 t -4173 e -4009 z 1537 d ( 1939 w ( 2541 n 410 w -609 c ( 1415 ) l ( -4017 w -4072 x ( 1378 r 1944 ) i 2279 v -1242 s -4521 l 1608 ) ) )", ops);
		requireExpressionEvaluatesTo("-3860 l 4024 z ( 3234 l ( 4344 e 4822 ) i 442 a ( 3360 t ( 39 j -3130 ) d -1676 f 702 n ( -551 b -1390 z -1932 k 1110 t ( -3654 z ( -4144 ) ) ) ) )", ops, -0.959252885060116);


	}
}