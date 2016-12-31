<elfscript.internal.test>
<unit.parse>
// line comment
/*
  block comment
  // test
  long
  whatever
  /*
*/
//* line comment
// /* line comment
basic = {
  delims = [
    [1; 2; 3;],
    [4; 5; 6 ],
    [1, 2, 3,],
    [4, 5, 6 ],
    [1; 2, 3 ],
    [4, 5; 6 ],
    {a = 1; b = 2; c = 3;},
    {a = 1; b = 3; c = 3 },
    {a = 1, b = 2, c = 3,},
    {a = 1, b = 2, c = 3 },
    {a = 1; b = 2, c = 3 },
    {1; 2; 3;},
    {a; b; c};
  ];
};
integers = [
  0;
  12;
  99947845612301310;
  -15;
  0xabcdef;
  0xa0b0c0d0e0f0;
  0x80ff90ffa0ffb0ff;
  0x80ff90ffa0ffb0ff00ff10ff20ff30ff;
  0x80ff90ffa0ffb0ff00ff10ff20ff30ff34567890;
  -0xff;
  0x0001;
];
numbers = [
  0.5;
  182.1922;
  1e7;
  1.4e3;
  .4e3;
  4.e3;
  0;
  1;
];
strings = [
  "test";
  'test';
  "string 'with embedded quotes'";
  'string "with embedded quotes"';
  "なまえ";
  "名前";
];
objects = [
  #rngtable {
    values = [
      1, 2,
      3,
      4, 5,
      6, 7
    ],
    weights = [
      0.40, 0.35, // 75% |  75%
      0.10,       // 10% |  85%
      0.05, 0.05, // 10% |  95%
      0.03, 0.02, //  5% | 100%
    ];
  };
];
units = [
  #parse_test { as = "int"; expect = "fail"; input = ""; },
  #parse_test { as = "int"; expect = "succeed"; input = "1"; output = 1; },
  #parse_test { as = "int"; expect = "succeed"; input = "0"; output = 0; },
  #parse_test { as = "int"; expect = "succeed"; input = "10"; output = 10; },
  #parse_test { as = "int"; expect = "succeed"; input = "23"; output = 23; },
  #parse_test { as = "int"; expect = "succeed"; input = "0x1"; output = 1; },
  #parse_test { as = "int"; expect = "succeed"; input = "0xa"; output = 10; },
  #parse_test { as = "int"; expect = "succeed"; input = "0x1a"; output = 26; },
  #parse_test { as = "int"; expect = "succeed"; input = "0x001"; output = 1; },
  #parse_test { as = "int"; expect = "succeed"; input = "0xff"; output = 255; },
  #parse_test { as = "int"; expect = "succeed"; input = "0xf9"; output = 249; },
  #parse_test { as = "int"; expect = "succeed"; input = "-3"; output = -3; },
  #parse_test { as = "int"; expect = "succeed"; input = "-0x03"; output = -3; },
  #parse_test { as = "int"; expect = "succeed"; input = "-0x10"; output = -16;},
  #parse_test { as = "int"; expect = "fail"; input = "00"; },
  #parse_test { as = "int"; expect = "fail"; input = "ff"; },
  #parse_test { as = "int"; expect = "fail"; input = "c9"; },
  #parse_test { as = "int"; expect = "fail"; input = "--3"; },
  #parse_test { as = "int"; expect = "fail"; input = "03"; },
  #parse_test { as = "int"; expect = "fail"; input = "-x35"; },
  #parse_test {
    as = "int"; expect = "remainder"; input = "0x"; output = 0; left = "x"
  },
  #parse_test {
    as = "int"; expect = "remainder"; input = "0x-5"; output = 0; left = "x-5"
  },
  #parse_test {
    as = "int"; expect = "remainder"; input = "19ab"; output = 19; left = "ab"
  },
  #parse_test {
    as = "int"; expect = "remainder"; input = "0-2"; output = 0; left = "-2"
  },
  #parse_test {
    as = "int"; expect = "remainder"; input = "0xff-"; output = 0xff; left = "-"
  },
  #parse_test {
    as = "int"; expect = "remainder"; input = "123-"; output = 123; left = "-"
  },
  #parse_test {
    as = "int";
    expect = "remainder";
    input = "120x456";
    output = 120;
    left = "x456";
  },
  #parse_test { as = "num"; expect = "succeed"; input = "0"; output = 0; }
  #parse_test { as = "num"; expect = "succeed"; input = "1"; output = 1; }
  #parse_test { as = "num"; expect = "succeed"; input = "0.0"; output = 0; }
  #parse_test { as = "num"; expect = "succeed"; input = "1.0"; output = 1; }
  #parse_test { as = "num"; expect = "succeed"; input = "1."; output = 1.0; }
  #parse_test { as = "num"; expect = "fail"; input = "0003.5"; }
  #parse_test {
    as = "num"; expect = "remainder"; input = "0.."; output = 0; left = "."
  },
  #parse_test {
    as = "num"; expect = "remainder"; input = "1e+-2"; output = 1; left = "e+-2"
  },
  #parse_test {
    as = "num"; expect = "remainder"; input = "1e-+2"; output = 1; left = "e-+2"
  },
  #parse_test {
    as = "num"; expect = "remainder"; input = "1e++2"; output = 1; left = "e++2"
  },
  #parse_test {
    as = "num";
    expect = "remainder";
    input = "-2e--2";
    output = -2;
    left = "e--2"
  },
  #parse_test {
    as = "num"; expect = "remainder"; input = "1+2"; output = 1; left = "+2"
  },
  #parse_test {
    as = "num"; expect = "remainder"; input = "1-2"; output = 1; left = "-2"
  },
  #parse_test {
    as = "num"; expect = "remainder"; input = "1.-2"; output = 1; left = "-2"
  },
  #parse_test {
    as = "string";
    expect = "success";
    input = "'string ''with embedded quotes'''";
    output = "string 'with embedded quotes'";
  },
  #parse_test {
    as = "string";
    expect = "success";
    input = '"string ""with embedded quotes"""';
    output = 'string "with embedded quotes"';
  },
  #parse_test {
    as = "assignment";
    expect = "success";
    input = "なまえ = '名前'";
  },
  #parse_test {
    as = "assignment";
    expect = "success";
    input = "なまえ = 名前";
  },
  #parse_test {
    as = "identifier";
    expect = "succeed";
    input = "a__very____long________name________________which________should____still__work_just_fine"
  },
  #parse_test { as = "expression"; expect = "succeed"; input = "1+2"; },
  #parse_test { as = "expression"; expect = "succeed"; input = "1 + 2"; },
  #parse_test { as = "expression"; expect = "succeed"; input = "1+2*3"; },
  #parse_test { as = "expression"; expect = "succeed"; input = "1 + 2 * 3"; },
  #parse_test { as = "expression"; expect = "succeed"; input = "5e7 + 0x5e7"; },
  #parse_test { as = "expression"; expect = "succeed"; input = "5e7 + e7"; },
  #parse_test { as = "expression"; expect = "succeed"; input = "-1+-2"; },
  #parse_test { as = "expression"; expect = "succeed"; input = "1--2"; },
  #parse_test { as = "expression"; expect = "succeed"; input = "(1 + 2) * 3"; },
  #parse_test { as = "expression"; expect = "succeed"; input = "(a+b)*(c+d)"; },
  #parse_test { as = "expression"; expect = "succeed"; input = "a**2 + b**2"; },
  #parse_test { as = "expression"; expect = "fail"; input = "1+2+"; },
  #parse_test { as = "expression"; expect = "fail"; input = "1+2-"; },
  #parse_test { as = "expression"; expect = "fail"; input = "+1+2"; },
  #parse_test { as = "expression"; expect = "fail"; input = "(1+2"; },
  #parse_test { as = "expression"; expect = "fail"; input = "1+2)"; },
  #parse_test {
    as = "object";
    expect = "succeed";
    input = "#parse_test {
      as = 'int';
      expect = 'succeed';
      input = '12';
      output = 12 }";
  },
];
</unit.parse>

<unit.general>
evaluation = [
  #eval_test { input = "5e7 + 0x5e7"; output = 50001511; },
  #eval_test { input = "1 + 1"; output = 2; },
  #eval_test { input = "2 + 2"; output = 4; },
  #eval_test { input = "1 * 1"; output = 1; },
  #eval_test { input = "1 + 5 * 7"; output = 36; },
  #eval_test { input = "1 + (5 * 7)"; output = 36; },
  #eval_test { input = "(1 + 5) * 7"; output = 42; },
  #eval_test { input = "2**2 * 3**3"; output = 108; },
  #eval_test { input = "2 * 2**3 * 3"; output = 48; },
  #eval_test { input = "2 * (2**3) * 3"; output = 48; },
  #eval_test { input = "(2 * 2)**(3 * 3)"; output = 18014398509481984; },
  #eval_test { input = "5 % 5"; output = 0; },
  #eval_test { input = "4 * 2%5"; output = 8; },
  #eval_test { input = "(4 * 2) % 5"; output = 3; },
  #eval_test { input = "1 + 4 % 5"; output = 5; },
  #eval_test { input = "(1 + 4) % 5"; output = 0; },
  #eval_test { input = "1 / 2"; output = 0.5; },
  #eval_test { input = "1 // 2"; output = 0; },
  #eval_test { input = "7 / 2"; output = 3.5; },
  #eval_test { input = "7 // 2"; output = 3; },
  #eval_test { input = "7.5 // 2.3"; output = 3; },
  #eval_test { input = "7.5 // 2.9"; output = 2; },
  #eval_test { input = "4**0.5"; output = 2; },
  #eval_test { input = "9**0.5"; output = 3; },
  #eval_test { input = "5 / 4 / 2"; output = 0.625; },
  #eval_test { input = "5 / (4 / 2)"; output = 2.5; },
  #eval_test { input = "5 / 4 * 2"; output = 2.5; },
  #eval_test { input = "5 / (4 * 2)"; output = 0.625; },
  #eval_test { input = "1 | 2"; output = 3; },
  #eval_test { input = "1 | 3"; output = 3; },
  #eval_test { input = "4 | 2 | 1"; output = 7; },
  #eval_test { input = "1 & 2"; output = 0; },
  #eval_test { input = "3 & 5"; output = 1; },
  #eval_test { input = "3 & 7"; output = 3; },
  #eval_test { input = "3 < 4"; output = True; },
  #eval_test { input = "3 <= 4"; output = True; },
  #eval_test { input = "4 <= 4"; output = True; },
  #eval_test { input = "5 <= 4"; output = False; },
  #eval_test { input = "5 < 4"; output = False; },
  #eval_test { input = "4 < 4"; output = False; },
  #eval_test { input = "6 >= 6"; output = True; },
  #eval_test { input = "6 >= -6"; output = True; },
  #eval_test { input = "6 > -6"; output = True; },
  #eval_test { input = "6 > 6"; output = False; },
  #eval_test { input = "-10 > 6"; output = False; },
  #eval_test { input = "-10 >= 6"; output = False; },
  #eval_test { input = "10 == 10"; output = True; },
  #eval_test { input = "1.0 == 1"; output = True; },
  #eval_test { input = "3 < 4 && 4 < 5"; output = True; },
  #eval_test { input = "3 > 4 && 4 < 5"; output = False; },
  #eval_test { input = "3 > 4 && 4 > 5"; output = False; },
  #eval_test { input = "3 < 4 || 4 < 5"; output = True; },
  #eval_test { input = "3 > 4 || 4 < 5"; output = True; },
  #eval_test { input = "3 > 4 || 4 > 5"; output = False; },
  #eval_test { input = "3 < 4 < 5"; output = True; },
  #eval_test { input = "4 < 4 < 5"; output = False; },
  #eval_test { input = "3 < 4 < 4"; output = False; },
  #eval_test { input = "4 < 4 < 4"; output = False; },
  #eval_test { input = "1 << 1"; output = 2; },
  #eval_test { input = "2 >> 1"; output = 1; },
  #eval_test { input = "1 << 3"; output = 8; },
  #eval_test { input = "0x10 >> 4"; output = 0x01; },
  #eval_test { input = "0x1f >> 4"; output = 1; },
  #eval_test { input = "0x1f >> 3"; output = 3; },
  #eval_test { input = "0x1f >> 2"; output = 7; },
  #eval_test { input = "0x10 >> 2"; output = 4; },
  #eval_test { input = "1 << 3 | 1 << 2 | 1 << 1"; output = 0xd; },
  #eval_test { input = "1 << 3 & 1 << 2 & 1 << 1"; output = 0; },
  #eval_test { input = "(1 << 3) | (1 << 2) | (1 << 1)"; output = 0xd; },
  #eval_test { input = "1 << (3 & 1) << (2 & 1) << 1"; output = 4; },
  #eval_test { input = "3 * 4 << 1"; output = 24; },
  #eval_test { input = "(3 * 4) << 1"; output = 24; },
  #eval_test { input = "3 * (4 << 1)"; output = 24; },
  #eval_test { input = "3 + 4 << 1"; output = 11; },
  #eval_test { input = "3 + (4 << 1)"; output = 11; },
  #eval_test { input = "(3 + 4) << 1"; output = 14; },
  #eval_test { input = "3 << 2 + 5"; output = 17; },
  #eval_test { input = "(3 << 2) + 5"; output = 17; },
  #eval_test { input = "3 << (2 + 5)"; output = 384; },
  #eval_test { input = "3 << 2 * 5"; output = 60; },
  #eval_test { input = "(3 << 2) * 5"; output = 60; },
  #eval_test { input = "3 << (2 * 5)"; output = 3072; },
  #eval_test { input = "3 << 4 / 4"; output = 12; },
  #eval_test { input = "(3 << 4) / 4"; output = 12; },
  #eval_test { input = "3 << (4 / 4)"; output = 6; },
  #eval_test { input = "3 >> 4 / 4"; output = 0; },
  #eval_test { input = "(3 >> 4) / 4"; output = 0; },
  #eval_test { input = "3 >> (4 / 4)"; output = 1; },
];
scope = {
  tf1 = !f (x = 0) {
    !assert(x == 0);
    x = 3;
    y = 5;
    nested = {
      !assert(x == 3);
      !assert(y == 5);
      x = 55;
      ~y = 17
      !assert(x == 55);
      !assert(y == 17);
    };
    nested2 = {
      !assert(x == 3);
      !assert(y == 17);
      ~y = -5;
      !assert(y == -5);
      y = 7;
      !assert(y == 7);
    };
    reassign = {
      !assert(x == 3);
      !assert(y == -5);
      x = 5;
      !assert(x == 5);
      x = 7;
      !assert(x == 7);
      x = 9;
      !assert(x == 9);
    }
    empty = {
    };
    { // an unnamed scope
      x = 231;
      y = 795;
      !assert(x == 231);
      !assert(y == 795);
      ~x = 3;
      !assert(x == 3);
    };
    // final assertions
    !assert(x == 3);
    !assert(y == -5);
    !assert(nested.x == 55);
    !assert(nested.y == -5);
    !assert(nested2.x == 3);
    !assert(nested2.y == -5);
    !assert(empty.x == 3);
    !assert(empty.y == -5);
    !return(x);
  };
  // call our test function
  tf1(0);
  tf1();
};
</unit.general>
</elfscript.internal.test>
