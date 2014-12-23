//
//  hangeulTests.m
//  hangeulTests
//
//  Created by Jeong YunWon on 2014. 6. 7..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#import <XCTest/XCTest.h>

#include <hangeul/hangeul.h>
#include <hangeul/hangul.h>

#include <hangeul/utf8.h>

using namespace hangeul;
using namespace hangeul::KSX5002;

@interface hangeulTests : XCTestCase

@end

@implementation hangeulTests

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testExample {
    //XCTFail(@"No implementation for \"%s\"", __PRETTY_FUNCTION__);
}

- (void)testOptional {
    auto opt1 = Optional<int>::None();
    XCTAssertEqual(opt1.is_none, true, @"");
    int v = 10;
    auto opt2 = Optional<int>::Some(v);
    XCTAssertEqual(opt2.is_none, false, @"");
    XCTAssertEqual(opt2.some, 10, @"");
}

- (void)testStrokePhase {
    QwertyToKeyStrokePhase phase;
    State s;
    s[-1] = 'r';
    auto r = phase.put(s);
    XCTAssertTrue(r.processed, @"");
    s = r.state;
    XCTAssertEqual(s[0], 20, @"");
}

- (void)testUnicode {
    XCTAssertEqual('a', UTF8('a').unicode(), @"");
    XCTAssertEqual('a', UTF8("a").unicode(), @"");
    XCTAssertEqual(0xac00, UTF8("가").unicode(), @"");
    XCTAssertEqual(0xd7a3, UTF8("힣").unicode(), @"");
    XCTAssertEqual(0xac00, UTF8::from_unicode(0xac00).unicode(), @"");
    XCTAssertEqual(0xd7a3, UTF8::from_unicode(0xd7a3).unicode(), @"");

    auto unicodes = UTF8::to_unicodes("한글 구름");
    XCTAssertEqual(unicodes[0], UTF8("한").unicode(), @"");
    XCTAssertEqual(unicodes[1], UTF8("글").unicode(), @"");
    XCTAssertEqual(unicodes[2], ' ', @"");
    XCTAssertEqual(unicodes[3], UTF8("구").unicode(), @"");
    XCTAssertEqual(unicodes[4], UTF8("름").unicode(), @"");

    auto roundtrip = UTF8::from_unicodes(unicodes);
    XCTAssertEqual(strcmp(roundtrip.c_str(), "한글 구름"), 0, @"");
    NSLog(@"%@", @(roundtrip.c_str()));
}

- (PhaseResult)testInput:(Phase&)phase :(State&)state :(char)input :(bool)processed :(size_t)size :(Consonant::Type) initial :(Vowel::Type)vowel :(Consonant::Type)final :(const char *)utf8data :(hangeul::Decoder&)decoder {
    state[-1] = input;
    auto r = phase.put(state);
    XCTAssertEqual(r.processed, processed, @"");
    //auto str = r.state.array(STRING_IDX);
    auto composed = decoder.composed(state);
    auto rstr = UTF8::from_unicodes(composed);
    XCTAssertTrue(rstr == utf8data, @"expected: %@ / result: %@", @(utf8data), @(rstr.c_str()));

//    if (r.state.size()) {
//        auto s = r.state.front();
//        XCTAssertEqual(r.state.size(), size, @"decoded: %@", @(UTF8::from_unicodes(decoder.decode(s)).c_str()));
//        XCTAssertEqual(s['a'], initial, @"value: %@ / expected: %@", @(UTF8::from_unicode(s['a']).chars), @(UTF8::from_unicode(initial).chars));
//        XCTAssertEqual(s['b'], vowel, @"");
//        XCTAssertEqual(s['c'], final, @"");
//        auto decoded = decoder.decode(s);
//        XCTAssertEqual(decoded[0], UTF8(utf8).unicode(), @"decoded: %@ / expected: %@", @(UTF8::from_unicodes(decoded).c_str()), @(utf8));
//    }
    return r;
}

- (void)testKSX5002Phase1 {
    // 구름 - 단순
    KSX5002::Combinator combinator;
    KSX5002::FromQwertyHandler phase(&combinator);
    KSX5002::Decoder decoder(&combinator);
    PhaseResult r;
    r = [self testInput:phase :r.state :'r' :true :1 :Consonant::G :Vowel::None :Consonant::None :"ㄱ" :decoder];
    r = [self testInput:phase :r.state :'n' :true :1 :Consonant::G :Vowel::U :Consonant::None :"구" :decoder];
    r = [self testInput:phase :r.state :'f' :true :1 :Consonant::G :Vowel::U :Consonant::R :"굴" :decoder];
    r = [self testInput:phase :r.state :'m' :true :2 :Consonant::R :Vowel::Eu :Consonant::None :"르" :decoder];
    r = [self testInput:phase :r.state :'a' :true :2 :Consonant::R :Vowel::Eu :Consonant::M :"름" :decoder];
    r = [self testInput:phase :r.state :' ' :false :3 :Consonant::None :Vowel::None :Consonant::None :" " :decoder];
}

- (void)testKSX5002Phase2 {
    // 왕까치는 없다.ㅏㄹ - 시프트, 겹받침
    KSX5002::Combinator combinator;
    KSX5002::FromQwertyHandler phase(&combinator);
    KSX5002::Decoder decoder(&combinator);
    PhaseResult r;

    r = [self testInput:phase :r.state :'d' :true :1 :Consonant::NG :Vowel::None :Consonant::None :"ㅇ" :decoder];
    r = [self testInput:phase :r.state :'h' :true :1 :Consonant::NG :Vowel::O :Consonant::None :"오" :decoder];
    r = [self testInput:phase :r.state :'k' :true :1 :Consonant::NG :Vowel::Wa :Consonant::None :"와" :decoder];
    r = [self testInput:phase :r.state :'d' :true :1 :Consonant::NG :Vowel::Wa :Consonant::NG :"왕" :decoder];
    r = [self testInput:phase :r.state :'R' :true :2 :Consonant::GG :Vowel::None :Consonant::None :"ㄲ" :decoder];
    r = [self testInput:phase :r.state :'k' :true :2 :Consonant::GG :Vowel::A :Consonant::None :"까" :decoder];
    r = [self testInput:phase :r.state :'c' :true :2 :Consonant::GG :Vowel::A :Consonant::CH :"깣" :decoder];
    r = [self testInput:phase :r.state :'l' :true :3 :Consonant::CH :Vowel::I :Consonant::None :"치" :decoder];
    r = [self testInput:phase :r.state :'s' :true :3 :Consonant::CH :Vowel::I :Consonant::N :"친" :decoder];
    r = [self testInput:phase :r.state :'m' :true :4 :Consonant::N :Vowel::Eu :Consonant::None :"느" :decoder];
    r = [self testInput:phase :r.state :'s' :true :4 :Consonant::N :Vowel::Eu :Consonant::N :"는" :decoder];
    r = [self testInput:phase :r.state :' ' :false :5 :Consonant::None :Vowel::None :Consonant::None :" " :decoder];

    r = [self testInput:phase :r.state :'d' :true :6 :Consonant::NG :Vowel::None :Consonant::None :"ㅇ" :decoder];
    r = [self testInput:phase :r.state :'j' :true :6 :Consonant::NG :Vowel::Eo :Consonant::None :"어" :decoder];
    r = [self testInput:phase :r.state :'q' :true :6 :Consonant::NG :Vowel::Eo :Consonant::B :"업" :decoder];
    r = [self testInput:phase :r.state :'t' :true :6 :Consonant::NG :Vowel::Eo :Consonant::BS :"없" :decoder];
    r = [self testInput:phase :r.state :'e' :true :7 :Consonant::D :Vowel::None :Consonant::None :"ㄷ" :decoder];
    r = [self testInput:phase :r.state :'k' :true :7 :Consonant::D :Vowel::A :Consonant::None :"다" :decoder];
    r = [self testInput:phase :r.state :'.' :false :8 :Consonant::None :Vowel::None :Consonant::None :"." :decoder];

    r = [self testInput:phase :r.state :'k' :true :9 :Consonant::None :Vowel::A :Consonant::None :"ㅏ" :decoder];
    r = [self testInput:phase :r.state :'f' :true :10 :Consonant::R :Vowel::None :Consonant::None :"ㄹ" :decoder];
}

- (void)testKSX5002Backspace {
    KSX5002::Combinator combinator;
    KSX5002::FromQwertyHandler phase(new KSX5002::Combinator());
    KSX5002::Decoder decoder(&combinator);
    PhaseResult r;

    r = [self testInput:phase :r.state :'k' :true :1 :Consonant::None :Vowel::A :Consonant::None :"ㅏ" :decoder];
    r = [self testInput:phase :r.state :0x7f :true :0 :Consonant::None :Vowel::None :Consonant::None :"" :decoder];
    r = [self testInput:phase :r.state :'d' :true :1 :Consonant::NG :Vowel::None :Consonant::None :"ㅇ" :decoder];
    r = [self testInput:phase :r.state :'h' :true :1 :Consonant::NG :Vowel::O :Consonant::None :"오" :decoder];
    r = [self testInput:phase :r.state :'k' :true :1 :Consonant::NG :Vowel::Wa :Consonant::None :"와" :decoder];
    r = [self testInput:phase :r.state :'d' :true :1 :Consonant::NG :Vowel::Wa :Consonant::NG :"왕" :decoder];
    r = [self testInput:phase :r.state :0x7f :true :1 :Consonant::NG :Vowel::Wa :Consonant::None :"와" :decoder];
    r = [self testInput:phase :r.state :'d' :true :1 :Consonant::NG :Vowel::Wa :Consonant::NG :"왕" :decoder];
    r = [self testInput:phase :r.state :'d' :true :2 :Consonant::NG :Vowel::None :Consonant::None :"ㅇ" :decoder];
    r = [self testInput:phase :r.state :0x7f :true :1 :Consonant::NG :Vowel::Wa :Consonant::NG :"왕" :decoder];

}

- (void)testKSX5002Context {
    KSX5002::Combinator combinator;
    KSX5002::FromQwertyHandler handler(new KSX5002::Combinator());
    KSX5002::Decoder decoder(&combinator);
    Context context(&handler, &combinator, &decoder);
    for (auto c: "rnfma") {
        if (c == 0) break;
        context.put(c);
        auto composed = context.composed();
        XCTAssertGreaterThan(composed.size(), 0, "composed: %@", @(UTF8::from_unicodes(composed).c_str()));
    }
    auto result = context.composed();
    XCTAssertEqual(result.size(), 1, @"size: %@", @(result.size()));
    XCTAssertEqual(result[0], UTF8("구").unicode(), @"result: %@", @(UTF8::from_unicodes(result).c_str()));
    context.flush();
    result = context.commited();
    XCTAssertEqual(result.size(), 1, @"");
    XCTAssertEqual(result[0], UTF8("름").unicode(), @"result: %@", @(UTF8::from_unicodes(result).c_str()));
}

- (void)testKSX5002Context2 {
    KSX5002::Combinator combinator;
    KSX5002::FromQwertyHandler handler(new KSX5002::Combinator());
    KSX5002::Decoder decoder(&combinator);
    Context context(&handler, &combinator, &decoder);
    for (auto c: "gksrmf rnfma dlqfur answkd") {
        if (c == 0) break;
        context.put(c);
        auto composed = context.composed();
        NSLog(@"composed: %@", @(UTF8::from_unicodes(composed).c_str()));
    }
    auto result = context.commited();
    XCTAssertEqual(result.size(), 10, @"");
    XCTAssertEqual(UTF8::from_unicodes(result), "한글 구름 입력 문", @"%@", @(UTF8::from_unicodes(result).c_str()));
    context.flush();
    result = context.commited();
    XCTAssertEqual(result.size(), 1, @"");
    XCTAssertEqual(result[0], UTF8("장").unicode(), @"%@", @(UTF8::from_unicodes(result).c_str()));
}

- (void)testCBinding {
//    void *c = _context();
//    auto r = _put(c, 'r');
//    r = _put(c, 'n');
//    r = _put(c, 'f');
}

@end
