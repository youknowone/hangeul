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
    StateList ss;
    auto r = phase.put_state(ss, State::InputSource('r'));
    XCTAssertTrue(r.processed, @"");
    ss = r.states;
    auto s = ss.front();
    XCTAssertEqual(s[2], 20, @"");
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

- (PhaseResult)testInput:(Phase&)phase :(StateList&)states :(char)input :(bool)processed :(size_t)size :(Consonant::Type) initial :(Vowel::Type)vowel :(Consonant::Type)final :(const char *)utf8 :(hangeul::Decoder&)decoder {
    auto is = State::InputSource(input);
    auto r = phase.put_state(states, is);
    XCTAssertEqual(r.processed, processed, @"");
    if (r.states.size()) {
        auto s = r.states.front();
        XCTAssertEqual(r.states.size(), size, @"decoded: %@", @(UTF8::from_unicodes(decoder.decode(s)).c_str()));
        XCTAssertEqual(s['a'], initial, @"value: %@ / expected: %@", @(UTF8::from_unicode(s['a']).chars), @(UTF8::from_unicode(initial).chars));
        XCTAssertEqual(s['b'], vowel, @"");
        XCTAssertEqual(s['c'], final, @"");
        auto decoded = decoder.decode(s);
        XCTAssertEqual(decoded[0], UTF8(utf8).unicode(), @"decoded: %@ / expected: %@", @(UTF8::from_unicodes(decoded).c_str()), @(utf8));
    }
    return r;
}

- (void)testKSX5002Phase1 {
    // 구름 - 단순
    KSX5002::FromQwertyPhase phase;
    KSX5002::Decoder decoder;
    PhaseResult r;
    r = [self testInput:phase :r.states :'r' :true :1 :Consonant::G :Vowel::None :Consonant::None :"ㄱ" :decoder];
    r = [self testInput:phase :r.states :'n' :true :1 :Consonant::G :Vowel::U :Consonant::None :"구" :decoder];
    r = [self testInput:phase :r.states :'f' :true :1 :Consonant::G :Vowel::U :Consonant::R :"굴" :decoder];
    r = [self testInput:phase :r.states :'m' :true :2 :Consonant::R :Vowel::Eu :Consonant::None :"르" :decoder];
    r = [self testInput:phase :r.states :'a' :true :2 :Consonant::R :Vowel::Eu :Consonant::M :"름" :decoder];
    r = [self testInput:phase :r.states :' ' :false :3 :Consonant::None :Vowel::None :Consonant::None :" " :decoder];
}

- (void)testKSX5002Phase2 {
    // 왕까치는 없다.ㅏㄹ - 시프트, 겹받침
    KSX5002::FromQwertyPhase phase;
    KSX5002::Decoder decoder;
    PhaseResult r;

    r = [self testInput:phase :r.states :'d' :true :1 :Consonant::NG :Vowel::None :Consonant::None :"ㅇ" :decoder];
    r = [self testInput:phase :r.states :'h' :true :1 :Consonant::NG :Vowel::O :Consonant::None :"오" :decoder];
    r = [self testInput:phase :r.states :'k' :true :1 :Consonant::NG :Vowel::Wa :Consonant::None :"와" :decoder];
    r = [self testInput:phase :r.states :'d' :true :1 :Consonant::NG :Vowel::Wa :Consonant::NG :"왕" :decoder];
    r = [self testInput:phase :r.states :'R' :true :2 :Consonant::GG :Vowel::None :Consonant::None :"ㄲ" :decoder];
    r = [self testInput:phase :r.states :'k' :true :2 :Consonant::GG :Vowel::A :Consonant::None :"까" :decoder];
    r = [self testInput:phase :r.states :'c' :true :2 :Consonant::GG :Vowel::A :Consonant::CH :"깣" :decoder];
    r = [self testInput:phase :r.states :'l' :true :3 :Consonant::CH :Vowel::I :Consonant::None :"치" :decoder];
    r = [self testInput:phase :r.states :'s' :true :3 :Consonant::CH :Vowel::I :Consonant::N :"친" :decoder];
    r = [self testInput:phase :r.states :'m' :true :4 :Consonant::N :Vowel::Eu :Consonant::None :"느" :decoder];
    r = [self testInput:phase :r.states :'s' :true :4 :Consonant::N :Vowel::Eu :Consonant::N :"는" :decoder];
    r = [self testInput:phase :r.states :' ' :false :5 :Consonant::None :Vowel::None :Consonant::None :" " :decoder];

    r = [self testInput:phase :r.states :'d' :true :6 :Consonant::NG :Vowel::None :Consonant::None :"ㅇ" :decoder];
    r = [self testInput:phase :r.states :'j' :true :6 :Consonant::NG :Vowel::Eo :Consonant::None :"어" :decoder];
    r = [self testInput:phase :r.states :'q' :true :6 :Consonant::NG :Vowel::Eo :Consonant::B :"업" :decoder];
    r = [self testInput:phase :r.states :'t' :true :6 :Consonant::NG :Vowel::Eo :Consonant::BS :"없" :decoder];
    r = [self testInput:phase :r.states :'e' :true :7 :Consonant::D :Vowel::None :Consonant::None :"ㄷ" :decoder];
    r = [self testInput:phase :r.states :'k' :true :7 :Consonant::D :Vowel::A :Consonant::None :"다" :decoder];
    r = [self testInput:phase :r.states :'.' :false :8 :Consonant::None :Vowel::None :Consonant::None :"." :decoder];

    r = [self testInput:phase :r.states :'k' :true :9 :Consonant::None :Vowel::A :Consonant::None :"ㅏ" :decoder];
    r = [self testInput:phase :r.states :'f' :true :10 :Consonant::R :Vowel::None :Consonant::None :"ㄹ" :decoder];
}

- (void)testKSX5002Backspace {
    KSX5002::FromQwertyPhase phase;
    KSX5002::Decoder decoder;
    PhaseResult r;

    r = [self testInput:phase :r.states :'k' :true :1 :Consonant::None :Vowel::A :Consonant::None :"ㅏ" :decoder];
    r = [self testInput:phase :r.states :0x7f :true :0 :Consonant::None :Vowel::None :Consonant::None :"" :decoder];
    r = [self testInput:phase :r.states :'d' :true :1 :Consonant::NG :Vowel::None :Consonant::None :"ㅇ" :decoder];
    r = [self testInput:phase :r.states :'h' :true :1 :Consonant::NG :Vowel::O :Consonant::None :"오" :decoder];
    r = [self testInput:phase :r.states :'k' :true :1 :Consonant::NG :Vowel::Wa :Consonant::None :"와" :decoder];
    r = [self testInput:phase :r.states :'d' :true :1 :Consonant::NG :Vowel::Wa :Consonant::NG :"왕" :decoder];
    r = [self testInput:phase :r.states :0x7f :true :1 :Consonant::NG :Vowel::Wa :Consonant::None :"와" :decoder];
    r = [self testInput:phase :r.states :'d' :true :1 :Consonant::NG :Vowel::Wa :Consonant::NG :"왕" :decoder];
    r = [self testInput:phase :r.states :'d' :true :2 :Consonant::NG :Vowel::None :Consonant::None :"ㅇ" :decoder];
    r = [self testInput:phase :r.states :0x7f :true :1 :Consonant::NG :Vowel::Wa :Consonant::NG :"왕" :decoder];

}

- (void)testKSX5002Context {
    Context context(new KSX5002::FromQwertyPhase(), new KSX5002::Decoder());
    for (auto c: "rnfma") {
        if (c == 0) break;
        context.put(c);
        auto composed = context.composed();
    }
    auto result = context.commited();
    XCTAssertEqual(result.size(), 1, @"");
    XCTAssertEqual(result[0], UTF8("구").unicode(), @"%@", @(UTF8::from_unicodes(result).c_str()));
    context.flush();
    result = context.commited();
    XCTAssertEqual(result.size(), 1, @"");
    XCTAssertEqual(result[0], UTF8("름").unicode(), @"%@", @(UTF8::from_unicodes(result).c_str()));
}

- (void)testKSX5002Context2 {
    Context context(new KSX5002::FromQwertyPhase(), new KSX5002::Decoder());
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
    void *c = _context();
    auto r = _put(c, 'r');
    r = _put(c, 'n');
    r = _put(c, 'f');
}

@end
