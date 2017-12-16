/*
 * KnowledgeBase.cpp
 *
 *  Created on: 2011/05/20
 *      Author: Hiroki Sudo
 */

#include "KnowledgeBase.h"

bool KnowledgeBase::LOGGING_FLAG = false;
int KnowledgeBase::ABSENT_LIMIT = 1;
uint32_t KnowledgeBase::CONTROLS = 0x00L;
int KnowledgeBase::buzz_length = 3;

bool KnowledgeBase::OMISSION_FLAG = false;
bool KnowledgeBase::OMISSION_A = false;
bool KnowledgeBase::OMISSION_B = false;
bool KnowledgeBase::OMISSION_C = false;
bool KnowledgeBase::OMISSION_D = false;
std::vector<Rule> KnowledgeBase::MEANING_SPACE;

KnowledgeBase::KnowledgeBase() {
    DIC_BLD = false;
    indexed = false;
    exception.clear();
}

KnowledgeBase::~KnowledgeBase() {
}

KnowledgeBase&
        KnowledgeBase::operator=(const KnowledgeBase& dst) {
    cat_indexer = dst.cat_indexer;

    sbox_buffer = dst.sbox_buffer;
    sentence_box = dst.sentence_box;
    sentenceDB = dst.sentenceDB;

    word_box = dst.word_box;
    wordDB = dst.wordDB;

    word_dic = dst.word_dic;
    DIC_BLD = dst.DIC_BLD;
    LOGGING_FLAG = dst.LOGGING_FLAG;
    ABSENT_LIMIT = dst.ABSENT_LIMIT;
    CONTROLS = dst.CONTROLS;
    buzz_length = dst.buzz_length;
    OMISSION_FLAG = dst.OMISSION_FLAG;
    OMISSION_A = dst.OMISSION_A;
    OMISSION_B = dst.OMISSION_B;
    OMISSION_C = dst.OMISSION_C;
    OMISSION_D = dst.OMISSION_D;

    return *this;
}

void
KnowledgeBase::clear(void) {
    cat_indexer.index_counter = 0;

    sbox_buffer.clear();
    sentenceDB.clear();
    sentence_box.clear();

    wordDB.clear();
    word_box.clear();

    word_dic.clear();
}

void
KnowledgeBase::send_box(Rule& mail) {
    if (mail.type == RULE_TYPE::NOUN) {
        word_box.push_back(mail);
    } else if (mail.type == RULE_TYPE::SENTENCE) {
        sentence_box.push_back(mail);
    }
}

void
KnowledgeBase::send_box(std::vector<Rule>& mails) {
    std::vector<Rule>::iterator it;
    it = mails.begin();
    while (it != mails.end()) {
        send_box(*it);
        it++;
    }
}

void
KnowledgeBase::send_db(Rule& mail) {
    if (mail.type == RULE_TYPE::NOUN) {
        wordDB.push_back(mail);
    } else if (mail.type == RULE_TYPE::SENTENCE) {
        sentenceDB.push_back(mail);
    }
}

void
KnowledgeBase::send_db(std::vector<Rule>& mails) {
    std::vector<Rule>::iterator it;
    it = mails.begin();
    while (it != mails.end()) {
        send_db(*it);
        it++;
    }
}

void
KnowledgeBase::unique(RuleDBType& DB) {
    RuleDBType::iterator it, it2;
    it = DB.begin();
    while (it != DB.end()) {
        it2 = it + 1;
        while (it2 != DB.end()) {
            if (*it == *it2) {
                it2 = DB.erase(it2);
            } else {
                it2++;
            }
        }

        it++;
    }
}

/*
 * ALGORITHM
 * Chunk
 * ・Chunkの結果はChunkを発生させる可能性がある
 * -新しい文規則が発生
 * ・Chunkの結果はMergeを発生させる可能性がある
 * --新しい単語規則が発生する
 * ・Chunkの結果はReplaceを発生させる可能性がある
 * --新しい単語規則が発生する
 *
 * Merge
 * ・Mergeの結果はChunkを発生させる可能性がある
 * --カテゴリが書き換えられた文規則が発生するため
 * ・Mergeの結果がMergeを発生させる可能性は無い
 * --単語規則の内部と外部が変わる事がないため
 * ・Mergeの結果がReplaceを発生させる可能性は無い
 * --単語の外部言語列が変化しないため
 *
 * Replace
 * ・Replaceの結果はChunkを発生させる可能性がある
 * --外部言語、内部言語が書き換えられた文規則が発生する
 * ・Replaceの結果がMergeを発生させることはない
 * --カテゴリが増えるわけではないため
 * ・Replaceの結果がReplaceを発生させることはない
 * --単語規則の外部言語列が増えないため
 *
 * Chunk:再帰処理が必要
 * Merge:決定的アルゴリズムでOK
 * Replace:決定的アルゴリズムでOK
 */

bool
KnowledgeBase::consolidate(void) {
    bool flag = true, flag1, flag2, flag3, flag4;
    while (flag) {
        flag = false;

        flag1 = chunk();
        flag2 = merge();
        flag3 = replace();

        flag = flag1 | flag2 | flag3;
    }

    if (sentence_box.size() != 0) {
        std::cerr << "UNCHECKED SENTENCE REMAINNED" << std::endl;
        for (int i = 0; i < sentence_box.size(); i++)
            std::cerr << sentence_box[i].to_s() << std::endl;
        if (flag1)
            std::cerr << "F1" << std::endl;
        if (flag2)
            std::cerr << "F2" << std::endl;
        if (flag3)
            std::cerr << "F3" << std::endl;

        throw;
    }

    send_db(sbox_buffer);
    send_db(word_box);

    if (CONTROLS & USE_OBLITERATION)
        obliterate();

    unique(sentenceDB);
    unique(wordDB);

    sbox_buffer.clear();
    sentence_box.clear();
    word_box.clear();

    return true;
}

bool
KnowledgeBase::chunk(void) {
    RuleDBType::iterator it;
    Rule unchecked_sent;
    bool chunked;
    bool total_chunked = false;

    //BOXがからになるまで
    while (sentence_box.size() != 0) {
        //chunk ベースルール取り出し,取り出した要素を削除
        unchecked_sent = sentence_box.back();
        sentence_box.pop_back();

        //全てのチェック済み文規則とChunkテスト
        chunked = chunking_loop(unchecked_sent, sentenceDB);
        total_chunked |= chunked;
        if (chunked)
            break;

        //全ての未チェック文規則とChunkテスト
        chunked = chunking_loop(unchecked_sent, sbox_buffer);
        total_chunked |= chunked;
        if (chunked)
            break;

        //もしChunkが発生してなかったなら
        sbox_buffer.push_back(unchecked_sent);
    } //未チェック規則が無くなれば終了

    return total_chunked;
}

bool
KnowledgeBase::chunking_loop(Rule& unchecked_sent, RuleDBType& checked_rules) {
    RuleDBType buffer;
    RuleDBType::iterator it;
    Rule logging_rule;
    bool chunked;

    chunked = false;
    it = checked_rules.begin();
    while (!chunked && it != checked_rules.end()) { //CHUNKが未発生 且つ 最後に行くまで
        //Chunk
        buffer.clear();
        //		if(abs(unchecked_sent.composition() - (*it).composition()) <= 1)
        buffer = chunking(unchecked_sent, *it);

        //結果解析
        if (buffer.size() == 0) { //未発生
            it++;
        } else { //発生
            if (LOGGING_FLAG)
                logging_rule = *it;

            send_box(buffer);

            //Chunkされた既存文規則を削除
            it = checked_rules.erase(it);

            chunked = true;
        }
    } //Chunkが発生したか、全既存規則とテスト終了

    //log取得
    if (chunked & LOGGING_FLAG) {
        LogBox::push_log("\n-->>CHUNK:");
        LogBox::push_log("**FROM");
        LogBox::push_log(unchecked_sent.to_s());
        LogBox::push_log(logging_rule.to_s());
        LogBox::push_log("**TO");

        std::vector<Rule>::iterator it_log;
        it_log = buffer.begin();
        for (; it_log != buffer.end(); it_log++) {
            LogBox::push_log((*it_log).to_s());
        }
        LogBox::push_log("<<--CHUNK");
    }

    return chunked;
}

/*
 # chunk!
 # Algorithm
 # 1．内部言語の差異の個数が1
 # 1.1 内部言語の差異は、(非変数, 変数)、または(非変数,非変数)の組み合わせのいずれか
 # 2．外部言語の前方一致部分、後方一致部分がいずれか空でない
 # 3．外部言語で一致部分を取り除いた時、差異部分がともに空でない
 # 4．外部言語の差異部分は、全て文字か、または変数1文字
 #
 # 1．上記条件が満たされるとき以下
 # 1．内部言語の差異部分が変数であるとき
 # 2．変数でない方の規則について、その非変数、また外部言語の差異部分を使い
 #    単語規則を生成する。この単語規則のカテゴリは、内部言語の差異部分が変数
 #    であった方の規則において、その変数がもつ外部言語でのカテゴリを使用する。
 #
 # 1．内部言語の差異部分がともに非変数であるとき
 # 2．それぞれの非変数内部言語差異要素、外部言語の差異部分を用いて
 #    単語規則を二つ生成する。この単語規則のカテゴリは未使用のカテゴリを
 #    生成したものを使用し、また二つの単語規則ともに同じものを使用する。
 # 3．元の規則に対して、差異部分を変数で置き換える。この変数のカテゴリは
 #    2で使用したカテゴリを用いる
 #
 # Chunkが発生した場合その瞬間に該当する規則は削除される
 #
 */
std::vector<Rule>
KnowledgeBase::chunking(Rule& src, Rule& dst) {
    //0: unchunkable
    //1: chunk type 1
    //2: chunk type 2

    enum CHUNK_TYPE {
        UNABLE, TYPE1, TYPE2
    };
    RuleDBType buf;
    buf.clear();

    //chunk のための分析
    //internal
    //サイズの同一性チェック
    if (src.internal.size() != dst.internal.size())
        return buf;

    //内部言語列差異数検索
    int idiff_index = 0;
    int idiff_counter = 0;
    for (int i = 0; idiff_counter <= 1 && i < src.internal.size(); i++) {
        if (src.internal[i] != dst.internal[i]) {
            idiff_index = i;
            idiff_counter++;
        }
    }
    if (idiff_counter != 1)
        return buf;

    //内部言語列差異要素は共に変数であることはない
    if (src.internal[idiff_index].is_var() && dst.internal[idiff_index].is_var())
        return buf;

    //外部言語検査
    //前方一致長の取得
    ExType::iterator src_it, dst_it;
    src_it = src.external.begin();
    dst_it = dst.external.begin();
    int fmatch_length = 0;
    while (src_it != src.external.end() && dst_it != dst.external.end()
            && *src_it == *dst_it) {
        fmatch_length++;
        src_it++;
        dst_it++;
    }

    //後方一致長の取得
    ExType::reverse_iterator src_rit, dst_rit;
    src_rit = src.external.rbegin();
    dst_rit = dst.external.rbegin();
    int rmatch_length = 0;
    while (src_rit != src.external.rend() && dst_rit != dst.external.rend()
            && *src_rit == *dst_rit) {
        rmatch_length++;
        src_rit++;
        dst_rit++;
    }

    //前・後方一致長が0でない
    if (fmatch_length + rmatch_length == 0)
        return buf;

    //一致長より外部言語列は長い（差異要素が必ず存在する）
    if (fmatch_length + rmatch_length >= src.external.size())
        return buf;
    if (fmatch_length + rmatch_length >= dst.external.size())
        return buf;

    //チャンクタイプ検査
    CHUNK_TYPE chunk_type = UNABLE;
    Rule base, targ;
    if (src.internal[idiff_index].is_var()
            && dst.internal[idiff_index].is_ind()) {
        base = src;
        targ = dst;
        chunk_type = TYPE2;
    } else if (src.internal[idiff_index].is_ind()
            && dst.internal[idiff_index].is_var()) {
        base = dst;
        targ = src;
        chunk_type = TYPE2;
    } else if (src.internal[idiff_index].is_ind()
            && dst.internal[idiff_index].is_ind()) {
        base = src;
        targ = dst;
        chunk_type = TYPE1;
    } else {
        std::cerr << "Illegal Chunk Type" << std::endl;
        throw "Illegal Chunk Type";
    }

    //chunk check : Base
    //TYPE2
    switch (chunk_type) {
        case TYPE2:
            //インターナルは変数と対象、変数のほうは外部が変数1つのみ、対象の方は全部記号
            if (base.external.size() != fmatch_length + rmatch_length + 1)
                return buf;

            if (!(base.external[fmatch_length].is_cat()))
                return buf;

            break;

        case TYPE1:
            //外部差異要素が全て記号
            for (int i = fmatch_length; i < base.external.size() - rmatch_length;
                    i++) {
                if (!(base.external[i].is_sym()))
                    return buf;
            }
            break;

        default:
            throw "unknow chunk type";
    }

    //chunk check: Target
    //外部差異要素が全て記号
    for (int i = fmatch_length; i < targ.external.size() - rmatch_length; i++) {
        if (!(targ.external[i].is_sym()))
            return buf;
    }

    //CHUNK処理
    Rule sent, noun1, noun2;
    switch (chunk_type) {
        case TYPE1:
        {
            int new_cat_id;

            //internal
            //noun
            new_cat_id = cat_indexer.generate();
            ExType noun1_ex, noun2_ex;

            for (int i = fmatch_length; i < base.external.size() - rmatch_length;
                    i++) {
                noun1_ex.push_back(base.external[i]);
            }
            for (int i = fmatch_length; i < targ.external.size() - rmatch_length;
                    i++) {
                noun2_ex.push_back(targ.external[i]);
            }

            //		ExType::iterator diff_bit, diff_eit;
            //		diff_bit = base.external.begin() + fmatch_length;
            //		diff_eit = base.external.begin() + (base.external.size() - rmatch_length);
            //		noun1_ex.insert(noun1_ex.end(), diff_bit, diff_eit);
            //		diff_bit = targ.external.begin() + fmatch_length;
            //		diff_eit = targ.external.begin() + (targ.external.size() - rmatch_length);
            //		noun2_ex.insert(noun2_ex.end(), diff_bit, diff_eit);

            noun1.set_noun(new_cat_id, base.internal[idiff_index], noun1_ex);
            noun2.set_noun(new_cat_id, targ.internal[idiff_index], noun2_ex);

            //sentence
            Element new_cat, new_var;
            new_cat.set_cat(idiff_index, new_cat_id);
            new_var.set_var(idiff_index, new_cat_id);

            sent = base;
            sent.internal[idiff_index] = new_var;
            sent.external.clear();

            ExType::iterator it;
            it = base.external.begin();
            while (it != base.external.begin() + fmatch_length) {
                sent.external.push_back(*it);
                it++;
            }

            sent.external.push_back(new_cat);

            std::vector<Element>::reverse_iterator rit;
            rit = base.external.rbegin();
            while (rit != base.external.rbegin() + rmatch_length) {
                sent.external.push_back(*rit);
                rit++;
            }

            buf.push_back(sent);
            buf.push_back(noun1);
            buf.push_back(noun2);
            break;
        }

        case TYPE2:
        {
            Rule noun;
            ExType noun_ex;

            for (int i = fmatch_length; i < targ.external.size() - rmatch_length;
                    i++) {
                noun_ex.push_back(targ.external[i]);
            }
            noun.set_noun(base.internal[idiff_index].cat, targ.internal[idiff_index],
                    noun_ex);

            buf.push_back(base);
            buf.push_back(noun);
            break;
        }
        default:
            std::cerr << "CHUNK PROC ERROR" << std::endl;
            throw "CHUNK PROC ERROR";
    }

    return buf;
}

bool
KnowledgeBase::merge(void) {
    RuleDBType::iterator it;
    bool occurrence = false;
    bool merged;

    it = word_box.begin();
    while (it != word_box.end()) {
        merged = merging(*it);
        occurrence |= merged;

        if (merged) {
            it = word_box.erase(it);
        } else {
            it++;
        }
    }

    return occurrence;
}

bool
KnowledgeBase::merging(Rule& src) {
    //word_boxを変更しなければイテレータは使える
    RuleDBType::iterator it;
    RuleDBType buf, sub_buf;

    //word 適合するか検索
    //Merge対象として適合したら、直接カテゴリを書き換えて、
    //書き換えられたカテゴリを収拾
    //※mergeで単語を書き換えた結果はReplaceに影響しない
    std::map<int, bool> unified_cat;

    //既存単語規則の被変更カテゴリの収拾
    collect_merge_cat(src, wordDB, unified_cat);

    //未検証単語規則の被変更カテゴリの収拾
    collect_merge_cat(src, word_box, unified_cat);

    //被変更カテゴリの数が0ならMergeは起こらない
    if (unified_cat.size() == 0)
        return false;

    if (LOGGING_FLAG) {
        LogBox::push_log("\n-->>MERGE:");
        LogBox::push_log(src.to_s());
        LogBox::push_log("**ABOUT");
        std::map<int, bool>::iterator it;
        std::vector<std::string> cat_vec;
        it = unified_cat.begin();
        for (; it != unified_cat.end(); it++) {
            cat_vec.push_back(
                    Prefices::CAT + ":" + boost::lexical_cast<std::string>((*it).first));
        }
        std::string ss = boost::algorithm::join(cat_vec, ",");
        LogBox::push_log(ss);
        LogBox::push_log("**TARGET");
    }

    //単語の書き換え
    //既存単語規則
    merge_noun_proc(src, wordDB, unified_cat);

    //未検証単語規則
    merge_noun_proc(src, word_box, unified_cat);

    //文規則の書き換え
    sub_buf = merge_sent_proc(src, sbox_buffer, unified_cat);
    buf.insert(buf.end(), sub_buf.begin(), sub_buf.end());

    // boxは直接書き換え
    sub_buf = merge_sent_proc(src, sentence_box, unified_cat);
    buf.insert(buf.end(), sub_buf.begin(), sub_buf.end());

    sub_buf = merge_sent_proc(src, sentenceDB, unified_cat);
    buf.insert(buf.end(), sub_buf.begin(), sub_buf.end());

    //カテゴリが書き換えられた文規則をBOXへ追加
    sentence_box.insert(sentence_box.end(), buf.begin(), buf.end());

    if (LOGGING_FLAG) {
        LogBox::push_log("<<--MERGE");
    }

    //単語規則でカテゴリの書き換えが発生していることから
    //Mergeが発生している
    return true;
}

void
KnowledgeBase::collect_merge_cat(Rule& src, std::vector<Rule>& words,
        std::map<int, bool>& unified_cat) {
    std::vector<Rule>::iterator it, sbit, seit, dbit, deit;
    it = words.begin();
    while (it != words.end()) {
        if (src.cat != (*it).cat && //要らないけど一応
                src.internal == (*it).internal && src.external == (*it).external) {
            unified_cat.insert(std::map<int, bool>::value_type((*it).cat, true));
        }
        it++;
    }
}

void
KnowledgeBase::merge_noun_proc(Rule& src, RuleDBType& DB,
        std::map<int, bool>& unified_cat) {
    RuleDBType::iterator it;
    it = DB.begin();
    while (it != DB.end()) {
        if (unified_cat.find((*it).cat) != unified_cat.end()) {
            if (LOGGING_FLAG) {
                LogBox::push_log("MERGE-> " + (*it).to_s());
            }

            (*it).cat = src.cat;

            if (LOGGING_FLAG) {
                LogBox::push_log("MERGE<- " + (*it).to_s());
            }
        }
        it++;
    }
}

KnowledgeBase::RuleDBType
KnowledgeBase::merge_sent_proc(Rule& base_word, RuleDBType& DB,
        std::map<int, bool>& unified_cat) {
    RuleDBType buf; //BOXへ送られる規則のバッファ
    RuleDBType::iterator it;
    Rule temp;
    bool modified;

    it = DB.begin();
    while (it != DB.end()) {
        modified = false;
        temp = *it;

        //内部に被変更カテゴリの変数があるか調べ、合ったら書き換える
        for (int i = 0; i < temp.internal.size(); i++) {
            if (temp.internal[i].is_var()
                    && unified_cat.find(temp.internal[i].cat) != unified_cat.end()) {
                if (LOGGING_FLAG) {
                    LogBox::push_log("MERGE-> " + (*it).to_s());
                }

                temp.internal[i].cat = base_word.cat;
                modified = true;
            }
        }

        //内部に変更があった場合
        if (modified) {
            //外部のカテゴリ変数も書き換える
            for (int j = 0; j < temp.external.size(); j++) {
                if (//find unified cat
                        temp.external[j].type == ELEM_TYPE::CAT_TYPE
                        && unified_cat.find(temp.external[j].cat) != unified_cat.end()) {
                    temp.external[j].cat = base_word.cat;
                }
            }
            if (LOGGING_FLAG) {
                LogBox::push_log("MERGE<- " + temp.to_s());
            }
        }

        if (modified) { //カテゴリの書き換えが発生している場合
            //バッファに書き換えられた規則を追加
            buf.push_back(temp);

            //書き換えられた元の規則をメインDBから削除
            it = DB.erase(it);
        } else { //カテゴリの書き換えが発生していない場合
            //検査対象の文規則イテレータを次に進める
            it++;
        }
    }

    return buf;
}

bool
KnowledgeBase::replace(void) {
    RuleDBType::iterator it;
    bool occurrence = false;
    bool flag1, flag2, flag3;

    it = word_box.begin();
    while (it != word_box.end()) {
        flag1 = flag2 = flag3 = false;
        if (LOGGING_FLAG) {
            LogBox::push_log("\n-->>REPLACE");
        }
        flag1 = replacing(*it, sbox_buffer);
        flag2 = replacing(*it, sentenceDB);
        flag3 = replacing(*it, sentence_box);

        occurrence = occurrence | flag1 | flag2 | flag3;
        if (flag1 || flag2 || flag3) {
            if (LOGGING_FLAG) {
                LogBox::push_log("USED WORD:");
                LogBox::push_log((*it).to_s());
                LogBox::push_log("<<--REPLACE");
            }
        } else {
            if (LOGGING_FLAG)
                LogBox::pop_log();
        }
        it++;
    }

    it = wordDB.begin();
    while (it != wordDB.end()) {
        flag1 = flag2 = flag3 = false;

        if (LOGGING_FLAG) {
            LogBox::push_log("\n-->>REPLACE");
        }

        flag1 = replacing(*it, sbox_buffer);
        flag2 = replacing(*it, sentence_box);
        //		flag3 = replacing(*it, sentenceDB);

        //		occurrence = occurrence | flag1 | flag2 | flag3;
        occurrence = occurrence | flag1 | flag2;
        if (flag1 || flag2 || flag3) {
            if (LOGGING_FLAG) {
                LogBox::push_log("USED WORD:");
                LogBox::push_log((*it).to_s());
                LogBox::push_log("<<--REPLACE");
            }
        } else {
            if (LOGGING_FLAG)
                LogBox::pop_log();
        }
        it++;
    }

    return occurrence;
}

bool
KnowledgeBase::replacing(Rule& word, RuleDBType& checking_sents) {
    RuleDBType::iterator it;
    RuleDBType buffer;

    int ematchb;
    int imatchp;

    bool imatched;
    bool ematched;
    bool replaced;
    bool total_match = false;

    //文規則をWordでReplaceできるか調べる
    it = checking_sents.begin();
    while (it != checking_sents.end()) {
        replaced = false;

        //ガード
        if ((*it).external.size() < word.external.size()) {
            it++;
            continue; //while
        }

        //internal check
        //文規則の内部にWordの内部言語があるか検査
        imatched = false;
        imatchp = 0;
        for (int i = 0; !imatched && i < (*it).internal.size(); i++) {
            //内部言語に一致部分がある場合
            if ((*it).internal[i] == word.internal.front()) {
                imatchp = i;
                imatched = true;
            }
        }
        //後処理
        if (!imatched) { //内部言語列に一致部分無し
            it++;
            continue; // while
        }

        //external check
        int ex_limit;
        ex_limit = (*it).external.size() - word.external.size();
        ematched = false;
        ematchb = 0;
        for (int i = 0; !ematched && i <= ex_limit; i++) {
            if (std::equal((*it).external.begin() + i,
                    (*it).external.begin() + i + word.external.size(),
                    word.external.begin())) {
                ematched = true;
                ematchb = i;
                break;
            }
        }
        //後処理
        if (!ematched) { //外部言語列に一致部分無し
            it++;
            continue; // while
        }

        //文規則内部言語に一致箇所がある場合
        if (imatched && ematched) {
            Element catvar, var;
            ExType::iterator eit;

            //内部言語置き換え用の変数インスタンス
            var.set_var(imatchp, word.cat);

            //外部言語置き換え用のカテゴリ
            catvar.set_cat(imatchp, word.cat);

            if (LOGGING_FLAG) {
                LogBox::push_log("REPLACE-> " + (*it).to_s());
            }

            //書き換え
            //内部
            (*it).internal[imatchp] = var;

            //外部
            eit = (*it).external.begin();
            eit = (*it).external.erase(eit + ematchb,
                    eit + ematchb + word.external.size());
            (*it).external.insert(eit, catvar);
            buffer.push_back(*it);

            if (LOGGING_FLAG) {
                LogBox::push_log("REPLACE<- " + (*it).to_s());
            }
            replaced = true;
            total_match |= replaced;
        }

        //文DBイテレータ
        if (replaced) {
            it = checking_sents.erase(it);
        } else { //次の文規則へ
            it++;
        }
    } //while

    send_box(buffer);

    return total_match;
}

void
KnowledgeBase::prohibited(KnowledgeBase::PatternType rules) {
    exception.push_back(rules);
}

bool
KnowledgeBase::obliterate(void) {
    std::vector<Rule>::iterator it1, it2;
    std::vector<Rule> buf;
    bool is_remained, occurrence = false;

    //単語規則削除
    it1 = wordDB.begin();
    while (it1 != wordDB.end()) {
        is_remained = true;
        it2 = it1 + 1;
        while (it2 != wordDB.end() && is_remained) {
            if ((*it1).cat == (*it2).cat && (*it1).internal == (*it2).internal
                    && (*it1).external.size() > (*it2).external.size()) {
                is_remained = false;
            }
            it2++;
        }

        if (is_remained) {
            buf.push_back(*it1);
        } else {
            if (LOGGING_FLAG) {
                LogBox::push_log("OBLITERATE->" + (*it1).to_s());
            }
        }
        it1++;
    }
    wordDB = buf;

    //後処理
    buf.clear();

    //文規則削除
    it1 = sentenceDB.begin();
    while (it1 != sentenceDB.end()) {
        is_remained = true;
        it2 = it1 + 1;
        while (is_remained && it2 != sentenceDB.end()) {
            bool same_in = true;
            { //internal equality check
                std::vector<Element>::iterator elem_it1, elem_it2;
                elem_it1 = (*it1).internal.begin();
                elem_it2 = (*it2).internal.begin();

                while (same_in && elem_it1 != (*it1).internal.end()
                        && elem_it2 != (*it2).internal.end()) {
                    if ((*elem_it1).is_ind() && (*elem_it2).is_ind()
                            && *elem_it1 == *elem_it2) {
                        same_in &= true;
                    } else if ((*elem_it1).is_var() && (*elem_it2).is_var()
                            && (*elem_it1).obj == (*elem_it2).obj
                            && (*elem_it1).cat == (*elem_it2).cat) {
                        same_in &= true;
                    } else {
                        same_in &= false;
                    }
                    elem_it1++;
                    elem_it2++;
                }
            }

            if (same_in && (*it1).external.size() >= (*it2).external.size()) {
                is_remained = false;
            }
            it2++;
        }

        if (is_remained) {
            buf.push_back(*it1);
        } else {
            if (LOGGING_FLAG)
                LogBox::push_log("OBLITERATE->" + (*it1).to_s());
        }
        it1++;
    }
    sentenceDB = buf;

    return false;
}

void
KnowledgeBase::build_word_index(void) {
    if (DIC_BLD)
        return;

    typedef std::multimap<int, Rule> ItemType;
    RuleDBType::iterator it;

    it = wordDB.begin();
    while (it != wordDB.end()) {
        if (word_dic.find((*it).cat) != word_dic.end()) {
            word_dic[(*it).cat].insert(
                    ItemType::value_type((*it).internal.front().obj, *it));
        } else {
            ItemType temp;
            temp.insert(ItemType::value_type((*it).internal.front().obj, *it));
            word_dic.insert(std::map<int, ItemType>::value_type((*it).cat, temp));
        }

        normal_word_dic.insert(
                std::multimap<Element, Rule>::value_type((*it).internal.front(),
                (*it)));

        it++;
    }

    DIC_BLD = true;
}

Rule
KnowledgeBase::fabricate(Rule& src1) {

    std::vector<PatternType> groundable_patterns;
    std::map<PATTERN_TYPE, std::vector<PatternType> > all_patterns;
    Rule src;
    int rand_index;
    src = src1;
    int DB = 0;
    PatternType target_pattern;
    bool clipping_fl = OMISSION_FLAG, clipped;

    all_patterns = construct_grounding_patterns(src);

    if (LOGGING_FLAG) {
        LogBox::push_log("\n-->>FABRICATE1:");
    }
    
    if (all_patterns[COMPLETE].size() != 0) {
//        std::cerr << "DDDDDDDDDDDD" << std::endl;
        std::vector<PatternType> sorted_patterns;
        std::vector<PatternType>::iterator sort_it;
        int pattern_length = 0;

        //合成度が高いルールを優先
        if (CONTROLS & ANTECEDE_COMPOSITION) {
            std::sort(all_patterns[COMPLETE].begin(), all_patterns[COMPLETE].end(),
                    VectorSizeSort());
            pattern_length = all_patterns[COMPLETE].front().size();
            sort_it = all_patterns[COMPLETE].begin();
            for (;
                    sort_it != all_patterns[COMPLETE].end()
                    && (*sort_it).size() == pattern_length; sort_it++) {
                sorted_patterns.push_back(*sort_it);
            }
            all_patterns[COMPLETE].swap(sorted_patterns);
        }
        rand_index = MT19937::irand() % all_patterns[COMPLETE].size();

        if (clipping_fl) {
            clipped = clipping(src, (all_patterns[COMPLETE])[rand_index], target_pattern);
        } else {
            target_pattern = (all_patterns[COMPLETE])[rand_index];
        }

        if (LOGGING_FLAG) {
            std::vector<Rule>::iterator deb_it;

            LogBox::push_log("**CONSTRUCT");
            LogBox::push_log("***->>USED_RULES");
            deb_it = (all_patterns[COMPLETE])[rand_index].begin();
            while (deb_it != (all_patterns[COMPLETE])[rand_index].end()) {
                LogBox::push_log((*deb_it).to_s());
                deb_it++;
            }
            LogBox::push_log("***<<-USED_RULES");
            if (clipped) {
                //clippingを使った時のlogを残す
                LogBox::push_log("***->>CLIPPING_RULES");
                deb_it = target_pattern.begin();
                while (deb_it != target_pattern.end()) {
                    LogBox::push_log((*deb_it).to_s());
                    deb_it++;
                }
                LogBox::push_log("***<<-CLIPPING_RULES");
            }
        }

        graund_with_pattern(src, target_pattern);
    } else if (all_patterns[ABSOLUTE].size() != 0) {
//        std::cerr << "DDDDDDDDDDDD2" << std::endl;
        if (LOGGING_FLAG) {
            LogBox::push_log("**ABSOLUTE");
        }

        rand_index = MT19937::irand() % all_patterns[ABSOLUTE].size();
        //ABSOLUTEにはclippingしない

        src = (all_patterns[ABSOLUTE])[rand_index].front();
    } else if (all_patterns[SEMICOMPLETE].size() != 0) {
//        std::cerr << "DDDDDDDDDDDD3" << std::endl;
        std::vector<PatternType> sorted_patterns;
        std::vector<PatternType>::iterator sort_it;
        int pattern_length = 0;

        //合成度が高いルールを優先
        if (CONTROLS & ANTECEDE_COMPOSITION) {
            std::sort(all_patterns[SEMICOMPLETE].begin(),
                    all_patterns[SEMICOMPLETE].end(), VectorSizeSort());
            pattern_length = all_patterns[SEMICOMPLETE].front().size();
            sort_it = all_patterns[SEMICOMPLETE].begin();
            for (;
                    sort_it != all_patterns[SEMICOMPLETE].end()
                    && (*sort_it).size() == pattern_length; sort_it++) {
                sorted_patterns.push_back(*sort_it);
            }
            all_patterns[SEMICOMPLETE].swap(sorted_patterns);
        }
        rand_index = MT19937::irand() % all_patterns[SEMICOMPLETE].size();

        if (LOGGING_FLAG) {
            std::vector<Rule>::iterator deb_it;

            LogBox::push_log("**SEMI CONSTRUCT");
            LogBox::push_log("***->>USED_RULES");
            deb_it = (all_patterns[SEMICOMPLETE])[rand_index].begin();
            while (deb_it != (all_patterns[SEMICOMPLETE])[rand_index].end()) {
                LogBox::push_log((*deb_it).to_s());
                deb_it++;
            }
            LogBox::push_log("***<<-USED_RULES");
        }
//std::cerr << "?FFFFFFFF" << std::endl;
        //completion
        PatternType use_pattern;
        PatternType::iterator it;

        use_pattern = (all_patterns[SEMICOMPLETE])[rand_index];
        for (it = use_pattern.begin(); it != use_pattern.end(); it++) {
            if ((*it).is_noun() && (*it).external.size() == 0) {
                ExType buzz;
                buzz = construct_buzz_word();
                (*it).external = buzz;

                if (CONTROLS & USE_ADDITION_OF_RANDOM_WORD) {
                    Rule keep_word;
                    int index;
                    for (int i = 0; i < use_pattern.front().internal.size(); i++) {
                        if ((*it).internal.front() == use_pattern.front().internal[i]) {
                            index = i;
                            break;
                        }
                    }
                    keep_word.set_noun((*it).cat, src.internal[index], buzz);
                    send_db(keep_word);
                    if (LOGGING_FLAG) {
                        LogBox::push_log("***->>KEPT THE COMP_RULE");
                        LogBox::push_log(keep_word.to_s());
                        LogBox::push_log("***<<-KEPT THE COMP_RULE");
                    }
                    DIC_BLD = false;
                    word_dic.clear();
                    build_word_index();
                }
                if (LOGGING_FLAG) {
                    LogBox::push_log("***->>COMP_RULE");
                    LogBox::push_log((*it).to_s());
                    LogBox::push_log("***<<-COMP_RULE");
                }
            }
        }
//std::cerr << "GGGGGGGGGGGG" << std::endl;
        if (clipping_fl) {
            clipped = clipping(src, use_pattern, target_pattern);
        } else {
            target_pattern = use_pattern;
        }
        if (LOGGING_FLAG) {
            if (clipped) {
                //clippingを使った時のlogを残す
                std::vector<Rule>::iterator deb_it;
                LogBox::push_log("***->>CLIPPING_RULES");
                deb_it = target_pattern.begin();
                while (deb_it != target_pattern.end()) {
                    LogBox::push_log((*deb_it).to_s());
                    deb_it++;
                }
                LogBox::push_log("***<<-CLIPPING_RULES");
            }
        }

        graund_with_pattern(src, target_pattern);
    } else {
//        std::cerr << "DDDDDDDDDDDD4" << std::endl;
        if (LOGGING_FLAG) {
            LogBox::push_log("**RANDOM");
        }

        ExType ex;
        ex = construct_buzz_word();
        src.external.swap(ex);

        if (CONTROLS & USE_ADDITION_OF_RANDOM_WORD) {
            send_db(src);
            if (LOGGING_FLAG) {
                LogBox::push_log("**KEPT THE RULE");
            }
        }
    }

    if (LOGGING_FLAG) {
        LogBox::push_log("**OUTPUT");
        LogBox::push_log(src.to_s());
        LogBox::push_log("<<--FABRICATE");
    }
//    std::cerr << "EEEEEEEEEEEEE" << std::endl;

    return src;
}

Rule
KnowledgeBase::pseudofabricate(Rule& src1) {

    std::vector<PatternType> groundable_patterns;
    std::map<PATTERN_TYPE, std::vector<PatternType> > all_patterns, all_patterns2;
    Rule src;
    int rand_index;
    src = src1;
    int DB = 0;

    all_patterns = construct_grounding_patterns(src);
    all_patterns2 = natural_construct_grounding_patterns(src);
    exception_filter(all_patterns, all_patterns2);

    if (LOGGING_FLAG) {
        //    LogBox::push_log("\n-->>pseudoFABRICATE:");
    }

    if (all_patterns[SEMICOMPLETE].size() != 0) {
        std::vector<PatternType> sorted_patterns;
        std::vector<PatternType>::iterator sort_it;
        int pattern_length = 0;

        //合成度が高いルールを優先
        if (CONTROLS & ANTECEDE_COMPOSITION) {
            std::sort(all_patterns[SEMICOMPLETE].begin(),
                    all_patterns[SEMICOMPLETE].end(), VectorSizeSort());
            pattern_length = all_patterns[SEMICOMPLETE].front().size();
            sort_it = all_patterns[SEMICOMPLETE].begin();
            for (;
                    sort_it != all_patterns[SEMICOMPLETE].end()
                    && (*sort_it).size() == pattern_length; sort_it++) {
                sorted_patterns.push_back(*sort_it);
            }
            all_patterns[SEMICOMPLETE].swap(sorted_patterns);
        }
        rand_index = MT19937::irand() % all_patterns[SEMICOMPLETE].size();

        if (LOGGING_FLAG) {
            std::vector<Rule>::iterator deb_it;

            //        LogBox::push_log("**SEMI CONSTRUCT");
            //        LogBox::push_log("***->>USED_RULES");
            deb_it = (all_patterns[SEMICOMPLETE])[rand_index].begin();
            while (deb_it != (all_patterns[SEMICOMPLETE])[rand_index].end()) {
                //             LogBox::push_log((*deb_it).to_s());
                deb_it++;
            }
            //           LogBox::push_log("***<<-USED_RULES");
        }

        //completion
        PatternType use_pattern;
        PatternType::iterator it;

        use_pattern = (all_patterns[SEMICOMPLETE])[rand_index];
        for (it = use_pattern.begin(); it != use_pattern.end(); it++) {
            if ((*it).is_noun() && (*it).external.size() == 0) {
                ExType buzz;
                buzz = construct_buzz_word();
                (*it).external = buzz;

                if (CONTROLS & USE_ADDITION_OF_RANDOM_WORD) {
                    Rule keep_word;
                    int index;
                    for (int i = 0; i < use_pattern.front().internal.size(); i++) {
                        if ((*it).internal.front() == use_pattern.front().internal[i]) {
                            index = i;
                            break;
                        }
                    }
                    keep_word.set_noun((*it).cat, src.internal[index], buzz);
                    //send_db(keep_word);
                    if (LOGGING_FLAG) {
                        //                       LogBox::push_log("***->>KEPT THE COMP_RULE");
                        //                       LogBox::push_log(keep_word.to_s());
                        //                       LogBox::push_log("***<<-KEPT THE COMP_RULE");
                    }
                }
                if (LOGGING_FLAG) {
                    //                   LogBox::push_log("***->>COMP_RULE");
                    //                 LogBox::push_log((*it).to_s());
                    //                 LogBox::push_log("***<<-COMP_RULE");
                }
            }
        }

        graund_with_pattern(src, use_pattern);
    } else {
        if (LOGGING_FLAG) {
            //         LogBox::push_log("**RANDOM");
        }

        ExType ex;
        ex = construct_buzz_word();
        src.external.swap(ex);

        if (CONTROLS & USE_ADDITION_OF_RANDOM_WORD) {
            //send_db(src);
            if (LOGGING_FLAG) {
                //             LogBox::push_log("**KEPT THE RULE");
            }
        }
    }

    if (LOGGING_FLAG) {
        //       LogBox::push_log("**OUTPUT");
        //       LogBox::push_log(src.to_s());
        //       LogBox::push_log("<<--pseudoFABRICATE");
    }

    return src;
}

Rule
KnowledgeBase::fabricate_min_len(Rule& src1) {
    std::vector<PatternType> groundable_patterns;
    std::vector<Rule> buf;
    std::map<PATTERN_TYPE, std::vector<PatternType> > all_patterns, all_patterns2;
    Rule src;
    src = src1;
    int DB = 0;
    KnowledgeBase::PatternType target_pattern;
    bool omission_changed = false;

    all_patterns = construct_grounding_patterns(src);
    all_patterns2 = natural_construct_grounding_patterns(src);
    exception_filter(all_patterns, all_patterns2);

    if (LOGGING_FLAG) {
        LogBox::push_log("\n-->>FABRICATE2:");
    }

    //まずAbsoluteをプッシュ
    for (int i = 0; i < all_patterns[ABSOLUTE].size(); i++) {
        buf.push_back(all_patterns[ABSOLUTE][i].front());
    }

    //次にCompleteをグラウンドしてプッシュ
    for (int i = 0; i < all_patterns[COMPLETE].size(); i++) {
        if (LOGGING_FLAG) {
            std::vector<Rule>::iterator deb_it;

            LogBox::push_log("**CONSTRUCT");
            LogBox::push_log("***->>USED_RULES");
            deb_it = (all_patterns[COMPLETE])[i].begin();
            while (deb_it != (all_patterns[COMPLETE])[i].end()) {
                LogBox::push_log((*deb_it).to_s());
                deb_it++;
            }
            LogBox::push_log("***<<-USED_RULES");
        }
        Rule grounded_rule;
        grounded_rule = src;
        if (OMISSION_FLAG) {
            omission_changed = clipping(src1, all_patterns[COMPLETE][i], target_pattern);
        } else {
            target_pattern = all_patterns[COMPLETE][i];
        }
        graund_with_pattern(grounded_rule, target_pattern);
        // graund_with_pattern(grounded_rule, all_patterns[COMPLETE][i]);
        buf.push_back(grounded_rule);
    }

    //最後にインベンションしてプッシュ
    for (int i = 0; i < all_patterns[SEMICOMPLETE].size(); i++) {
        if (LOGGING_FLAG) {
            std::vector<Rule>::iterator deb_it;
            LogBox::push_log("**SEMI CONSTRUCT");
            LogBox::push_log("***->>USED_RULES");
            deb_it = (all_patterns[SEMICOMPLETE])[i].begin();
            while (deb_it != (all_patterns[SEMICOMPLETE])[i].end()) {
                LogBox::push_log((*deb_it).to_s());
                deb_it++;
            }
            LogBox::push_log("***<<-USED_RULES");
        }

        //completion
        PatternType use_pattern;
        PatternType::iterator it;

        use_pattern = all_patterns[SEMICOMPLETE][i];

        for (it = use_pattern.begin(); it != use_pattern.end(); it++) {
            if ((*it).is_noun() && (*it).external.size() == 0) {
                ExType buzz;
                buzz = construct_buzz_word();
                (*it).external = buzz;

                if (CONTROLS & USE_ADDITION_OF_RANDOM_WORD) {
                    Rule keep_word;
                    int index;
                    for (int i = 0; i < use_pattern.front().internal.size(); i++) {
                        if ((*it).internal.front() == use_pattern.front().internal[i]) {
                            index = i;
                            break;
                        }
                    }
                    keep_word.set_noun((*it).cat, src.internal[index], buzz);
                    send_db(keep_word);
                    if (LOGGING_FLAG) {
                        LogBox::push_log("***->>KEPT THE COMP_RULE");
                        LogBox::push_log(keep_word.to_s());
                        LogBox::push_log("***<<-KEPT THE COMP_RULE");
                    }
                    DIC_BLD = false;
                    word_dic.clear();
                    build_word_index();
                }

                if (LOGGING_FLAG) {
                    LogBox::push_log("***->>COMP_RULE");
                    LogBox::push_log((*it).to_s());
                    LogBox::push_log("***<<-COMP_RULE");
                }
            }
        }

        Rule grounded_rule;
        grounded_rule = src;
        if (OMISSION_FLAG) {
            omission_changed = clipping(src1, use_pattern, target_pattern);
        } else {
            target_pattern = use_pattern;
        }
        graund_with_pattern(grounded_rule, target_pattern);
        //        graund_with_pattern(grounded_rule, use_pattern);
        buf.push_back(grounded_rule);
    }

    //最後にもし発話できないときランダムをプッシュ
    if (buf.size() == 0) {
        if (LOGGING_FLAG) {
            LogBox::push_log("**RANDOM");
        }

        ExType ex;
        ex = construct_buzz_word();

        Rule grounded_rule;
        grounded_rule = src;

        grounded_rule.external = ex;
        buf.push_back(grounded_rule);

        if (CONTROLS & USE_ADDITION_OF_RANDOM_WORD) {
            send_db(src);
            if (LOGGING_FLAG) {
                LogBox::push_log("**KEPT THE RULE");
            }
        }
    }

    //でbufをExternalサイズでソート？
    std::sort(buf.begin(), buf.end(), ExternalSizeSort());

    if (LOGGING_FLAG) {
        LogBox::push_log("**OUTPUT");
        LogBox::push_log(buf.front().to_s());
        LogBox::push_log("<<--FABRICATE");
    }

    return buf.front();
}

KnowledgeBase::ExType
KnowledgeBase::construct_buzz_word(void) {
    //ランダム生成
    int length;
    int sym_id;
    std::vector<Element> ex;
    std::vector<Element> sym_buf;

    length = (MT19937::irand() % buzz_length) + 1;
    for (int i = 0; i < length; i++) {
        Element sym_buf;
        sym_id = MT19937::irand() % Dictionary::symbol.size();
        sym_buf.set_sym(sym_id);
        ex.push_back(sym_buf);
    }

    if (ex.size() == 0) {
        std::cout << "Failed making random" << std::endl;
        throw "make random external error";
    }
    return ex;
}

void
KnowledgeBase::graund_with_pattern(Rule& src, PatternType& pattern) {
    ExType::iterator sent_ex_it;
    PatternType::iterator pattern_it;
    Rule base_rule;
    PatternType tmp_ptn;

    tmp_ptn = pattern;
    base_rule = tmp_ptn.front();
    //tmp_ptn.erase(tmp_ptn.begin(), tmp_ptn.begin() + 1);
    tmp_ptn.erase(tmp_ptn.begin());
    sent_ex_it = base_rule.external.begin();
    src.external.clear();

    while (sent_ex_it != base_rule.external.end()) {
        if ((*sent_ex_it).is_sym()) {
            src.external.push_back(*sent_ex_it);
        } else if ((*sent_ex_it).is_cat()) {
            pattern_it = tmp_ptn.begin();
            while (pattern_it != tmp_ptn.end()) {
                if ((*pattern_it).internal.front().obj == (*sent_ex_it).obj
                        && (*pattern_it).cat == (*sent_ex_it).cat) {
                    src.external.insert(src.external.end(),
                            (*pattern_it).external.begin(), (*pattern_it).external.end());
                    break;
                }
                pattern_it++;
            }
        } else {
            std::cerr << "fabricate error" << std::endl;
            throw "fabricate error";
        }
        sent_ex_it++;
    }
}

std::map<KnowledgeBase::PATTERN_TYPE, std::vector<KnowledgeBase::PatternType> >
KnowledgeBase::construct_grounding_patterns(Rule& src) {
    typedef std::pair<std::multimap<int, Rule>::iterator,
            std::multimap<int, Rule>::iterator> DictionaryRange;

    /*
     * Srcに対して、それぞれの文規則がグラウンド可能か検査する
     * グラウンディング可能な場合、そのグラウンディングに使用する
     * 単語規則とその文規則の組の全パターンを集める
     */
    build_word_index();

    //SentenceDBシーケンス用
    RuleDBType::iterator sent_it;
    bool filted;
    int ungrounded_variable_num;
    bool is_applied, is_absorute, is_complete, is_semicomplete;
    std::map<PATTERN_TYPE, std::vector<KnowledgeBase::PatternType> > ret;
    //グラウンドパターンの格納庫とそのイテレータ
    std::vector<PatternType> patternDB;
    std::vector<PatternType>::iterator patternDB_it;
    //初期パターン
    PatternType pattern;
    //  exception検知用
    //  std::vector<std::vector<Rule> > exceptionDB;
    //  std::vector<std::vector<Rule> >::iterator exceptionDB_it;
    //  std::vector<Rule>::iterator exceptionP_it;
    //  bool exp_flag;

    sent_it = sentenceDB.begin();
    while (sent_it != sentenceDB.end()) {

        ungrounded_variable_num = 0;

        //拡張用:内部言語列長の同一性検査
        // 将来的に内部言語列長が異なるものがデータベースに入るかも知れないので
        if ((*sent_it).internal.size() != src.internal.size()) {
            sent_it++;
            continue;
        }

        //高速化枝狩り
        //対象が1カ所でも一致していないものは使えない
        filted = true;
        for (int index = 0; index < src.internal.size() && filted; index++) {
            if ((*sent_it).internal[index].is_ind()
                    && src.internal[index] != (*sent_it).internal[index]) {
                filted = false;
            }
        }
        if (!filted) {
            sent_it++;
            continue;
        }

        //    文ルールがマッチするexceptionだけをexceptionDBに格納
        //    exceptionDB_it=exception.begin();
        //    for(; exceptionDB_it!=exception.end(); exceptionDB_it++){
        //        exp_flag=((*exceptionDB_it)[0]==(*sent_it));
        //        
        //        if(exp_flag){
        //            exceptionDB.push_back((*exceptionDB_it));
        //        }
        //    }

        //グラウンドパターンの格納庫とそのイテレータ
        patternDB.clear();

        //初期パターン
        pattern.clear();

        //始めに検索対象文規則をパターン格納庫に入れる
        pattern.push_back(*sent_it);
        patternDB.push_back(pattern);

        //ある単語規則に対するグラウンドパターン検索
        Element grnd_elm, mean_elm;

        is_applied = is_absorute = is_complete = is_semicomplete = true;
        for (int in_idx = 0; is_applied && in_idx < (*sent_it).internal.size();
                in_idx++) {
            grnd_elm = (*sent_it).internal[in_idx]; //検査するインターナル要素
            mean_elm = src.internal[in_idx]; //基準のインターナル要素

            if (grnd_elm == mean_elm) { //単語がそのまま一致する場合
                is_absorute &= true;
                continue;
            } else if (//変数の場合で、グラウンド可能な場合
                    grnd_elm.is_var() && //変数で
                    word_dic.find(grnd_elm.cat) != word_dic.end() && //変数のカテゴリが辞書に有り
                    word_dic[grnd_elm.cat].find(mean_elm.obj)
                    != word_dic[grnd_elm.cat].end() //辞書の指定カテゴリに単語がある
                    ) {
                DictionaryRange item_range;
                std::vector<PatternType> patternDB_buffer;

                //変数に適用可能単語規則集合取得
                item_range = word_dic[grnd_elm.cat].equal_range(mean_elm.obj);

                //すでに作られてる単語組に対し組み合わせの直積の生成
                patternDB_it = patternDB.begin();
                while (patternDB_it != patternDB.end()) {
                    //検索した適用可能な単語規則列
                    while (item_range.first != item_range.second) {
                        Rule word_item;
                        PatternType sub_pattern;

                        //取得した単語規則をコピーして
                        word_item = (*(item_range.first)).second;

                        //変数用の単語規則をinternalに書き込み
                        word_item.internal.front().set_var(in_idx, grnd_elm.cat);

                        //すでに作られてる単語規則の組をコピー
                        sub_pattern = *patternDB_it;
                        //そこへ新しく単語規則を追加
                        sub_pattern.push_back(word_item);

                        //新しく単語規則が追加された単語規則組を新しく保存
                        patternDB_buffer.push_back(sub_pattern);

                        //適用可能単語規則列の最後まで繰り返す
                        item_range.first++;
                    }
                    //単語規則組が無くなるまで繰り返す
                    patternDB_it++;
                }
                //変更された単語規則組の列で元の単語規則組の列を置き換える
                patternDB.swap(patternDB_buffer);

                is_complete &= true;
                is_absorute &= false;
            } else { //適合不可能文規則
                ungrounded_variable_num++;

                if (ungrounded_variable_num > ABSENT_LIMIT) {
                    is_applied &= false;
                } else {
                    if (grnd_elm.is_var()) {
                        std::vector<PatternType> patternDB_buffer;

                        //すでに作られてる単語組に対し組み合わせの直積の生成
                        patternDB_it = patternDB.begin();
                        while (patternDB_it != patternDB.end()) {
                            //検索した適用可能な単語規則列
                            PatternType sub_pattern;
                            Rule empty_word;
                            ExType empty_ex;

                            //空の単語規則を作る
                            empty_word.set_noun(grnd_elm.cat, grnd_elm, empty_ex);

                            //すでに作られてる単語規則の組をコピー
                            sub_pattern = *patternDB_it;

                            //そこへ新しく単語規則を追加
                            sub_pattern.push_back(empty_word);

                            //新しく単語規則が追加された単語規則組を新しく保存
                            patternDB_buffer.push_back(sub_pattern);

                            //単語規則組が無くなるまで繰り返す
                            patternDB_it++;
                        }
                        //変更された単語規則組の列で元の単語規則組の列を置き換える
                        patternDB.swap(patternDB_buffer);
                    } else {
                        std::cerr << "pattern error" << std::endl;
                        throw;
                    }

                    is_absorute &= false;
                    is_complete &= false;
                    is_semicomplete &= true;
                }
            }
        } //内部言語のグラウンドループ

        //ある文規則に対して取得できたグラウンドパターンを保存
        if (is_applied) {
            if (is_absorute) {
                ret[ABSOLUTE].insert(ret[ABSOLUTE].end(), patternDB.begin(),
                        patternDB.end());
            } else if (is_complete) {
                ret[COMPLETE].insert(ret[COMPLETE].end(), patternDB.begin(),
                        patternDB.end());
            } else if (is_semicomplete) {
                ret[SEMICOMPLETE].insert(ret[SEMICOMPLETE].end(), patternDB.begin(),
                        patternDB.end());
            }
        }

        //次の文規則を検査
        sent_it++;
    } //文規則のループ

    //生成パターンの分類
    //	g_it = g_pattern.begin();
    //	for(; g_it != g_pattern.end(); g_it++){
    //		if((*g_it).front().composition() == 0){
    //			a_pattern.push_back(*g_it);
    //		}
    //		if((*g_it).front().composition()+1 == (*g_it).size()){
    //			c_pattern.push_back(*g_it);
    //		}
    //		else{
    //			s_pattern.push_back(*g_it);
    //		}
    //	}
    //
    //	ret[ABSOLUTE] = a_pattern;
    //	ret[COMPLETE] = c_pattern;
    //	ret[SEMICOMPLETE] = s_pattern;
    return ret;
}

std::map<KnowledgeBase::PATTERN_TYPE, std::vector<KnowledgeBase::PatternType> >
KnowledgeBase::natural_construct_grounding_patterns(Rule& src) {
    typedef std::pair<std::multimap<int, Rule>::iterator,
            std::multimap<int, Rule>::iterator> DictionaryRange;

    /*
     * Srcに対して、それぞれの文規則がグラウンド可能か検査する
     * グラウンディング可能な場合、そのグラウンディングに使用する
     * 単語規則とその文規則の組の全パターンを集める
     */
    //build_word_index();

    //SentenceDBシーケンス用
    RuleDBType::iterator sent_it;
    bool filted;
    int ungrounded_variable_num;
    bool is_applied, is_absorute, is_complete, is_semicomplete;
    std::map<PATTERN_TYPE, std::vector<KnowledgeBase::PatternType> > ret;
    //グラウンドパターンの格納庫とそのイテレータ
    std::vector<PatternType> patternDB;
    std::vector<PatternType>::iterator patternDB_it;
    //初期パターン
    PatternType pattern;
    //  exception検知用
    //  std::vector<std::vector<Rule> > exceptionDB;
    //  std::vector<std::vector<Rule> >::iterator exceptionDB_it;
    //  std::vector<Rule>::iterator exceptionP_it;
    //  bool exp_flag;

    sent_it = sentenceDB.begin();
    while (sent_it != sentenceDB.end()) {

        ungrounded_variable_num = 0;

        //拡張用:内部言語列長の同一性検査
        // 将来的に内部言語列長が異なるものがデータベースに入るかも知れないので
        if ((*sent_it).internal.size() != src.internal.size()) {
            sent_it++;
            continue;
        }

        //高速化枝狩り
        //対象が1カ所でも一致していないものは使えない
        filted = true;
        for (int index = 0; index < src.internal.size() && filted; index++) {
            if ((*sent_it).internal[index].is_ind()
                    && src.internal[index] != (*sent_it).internal[index]) {
                filted = false;
            }
        }
        if (!filted) {
            sent_it++;
            continue;
        }

        //    文ルールがマッチするexceptionだけをexceptionDBに格納
        //    exceptionDB_it=exception.begin();
        //    for(; exceptionDB_it!=exception.end(); exceptionDB_it++){
        //        exp_flag=((*exceptionDB_it)[0]==(*sent_it));
        //        
        //        if(exp_flag){
        //            exceptionDB.push_back((*exceptionDB_it));
        //        }
        //    }

        //グラウンドパターンの格納庫とそのイテレータ
        patternDB.clear();

        //初期パターン
        pattern.clear();

        //始めに検索対象文規則をパターン格納庫に入れる
        pattern.push_back(*sent_it);
        patternDB.push_back(pattern);

        //ある単語規則に対するグラウンドパターン検索
        Element grnd_elm, mean_elm;

        is_applied = is_absorute = is_complete = is_semicomplete = true;
        for (int in_idx = 0; is_applied && in_idx < (*sent_it).internal.size();
                in_idx++) {
            grnd_elm = (*sent_it).internal[in_idx]; //検査するインターナル要素
            mean_elm = src.internal[in_idx]; //基準のインターナル要素

            if (grnd_elm == mean_elm) { //単語がそのまま一致する場合
                is_absorute &= true;
                continue;
            } else if (//変数の場合で、グラウンド可能な場合
                    grnd_elm.is_var() && //変数で
                    word_dic.find(grnd_elm.cat) != word_dic.end() && //変数のカテゴリが辞書に有り
                    word_dic[grnd_elm.cat].find(mean_elm.obj)
                    != word_dic[grnd_elm.cat].end() //辞書の指定カテゴリに単語がある
                    ) {
                DictionaryRange item_range;
                std::vector<PatternType> patternDB_buffer;

                //変数に適用可能単語規則集合取得
                item_range = word_dic[grnd_elm.cat].equal_range(mean_elm.obj);

                //すでに作られてる単語組に対し組み合わせの直積の生成
                patternDB_it = patternDB.begin();
                while (patternDB_it != patternDB.end()) {
                    //検索した適用可能な単語規則列
                    while (item_range.first != item_range.second) {
                        Rule word_item;
                        PatternType sub_pattern;

                        //取得した単語規則をコピーして
                        word_item = (*(item_range.first)).second;

                        //変数用の単語規則をinternalに書き込み
                        //            word_item.internal.front().set_var(in_idx, grnd_elm.cat);

                        //すでに作られてる単語規則の組をコピー
                        sub_pattern = *patternDB_it;
                        //そこへ新しく単語規則を追加
                        sub_pattern.push_back(word_item);

                        //新しく単語規則が追加された単語規則組を新しく保存
                        patternDB_buffer.push_back(sub_pattern);

                        //適用可能単語規則列の最後まで繰り返す
                        item_range.first++;
                    }
                    //単語規則組が無くなるまで繰り返す
                    patternDB_it++;
                }
                //変更された単語規則組の列で元の単語規則組の列を置き換える
                patternDB.swap(patternDB_buffer);

                is_complete &= true;
                is_absorute &= false;
            } else { //適合不可能文規則
                ungrounded_variable_num++;

                if (ungrounded_variable_num > ABSENT_LIMIT) {
                    is_applied &= false;
                } else {
                    if (grnd_elm.is_var()) {
                        std::vector<PatternType> patternDB_buffer;

                        //すでに作られてる単語組に対し組み合わせの直積の生成
                        patternDB_it = patternDB.begin();
                        while (patternDB_it != patternDB.end()) {
                            //検索した適用可能な単語規則列
                            PatternType sub_pattern;
                            Rule empty_word;
                            ExType empty_ex;

                            //空の単語規則を作る
                            //              empty_word.set_noun(grnd_elm.cat, grnd_elm, empty_ex);

                            //すでに作られてる単語規則の組をコピー
                            sub_pattern = *patternDB_it;

                            //そこへ新しく単語規則を追加
                            sub_pattern.push_back(empty_word);

                            //新しく単語規則が追加された単語規則組を新しく保存
                            patternDB_buffer.push_back(sub_pattern);

                            //単語規則組が無くなるまで繰り返す
                            patternDB_it++;
                        }
                        //変更された単語規則組の列で元の単語規則組の列を置き換える
                        patternDB.swap(patternDB_buffer);
                    } else {
                        std::cerr << "pattern error" << std::endl;
                        throw;
                    }

                    is_absorute &= false;
                    is_complete &= false;
                    is_semicomplete &= true;
                }
            }
        } //内部言語のグラウンドループ

        //ある文規則に対して取得できたグラウンドパターンを保存
        if (is_applied) {
            if (is_absorute) {
                ret[ABSOLUTE].insert(ret[ABSOLUTE].end(), patternDB.begin(),
                        patternDB.end());
            } else if (is_complete) {
                ret[COMPLETE].insert(ret[COMPLETE].end(), patternDB.begin(),
                        patternDB.end());
            } else if (is_semicomplete) {
                ret[SEMICOMPLETE].insert(ret[SEMICOMPLETE].end(), patternDB.begin(),
                        patternDB.end());
            }
        }

        //次の文規則を検査
        sent_it++;
    } //文規則のループ

    //生成パターンの分類
    //	g_it = g_pattern.begin();
    //	for(; g_it != g_pattern.end(); g_it++){
    //		if((*g_it).front().composition() == 0){
    //			a_pattern.push_back(*g_it);
    //		}
    //		if((*g_it).front().composition()+1 == (*g_it).size()){
    //			c_pattern.push_back(*g_it);
    //		}
    //		else{
    //			s_pattern.push_back(*g_it);
    //		}
    //	}
    //
    //	ret[ABSOLUTE] = a_pattern;
    //	ret[COMPLETE] = c_pattern;
    //	ret[SEMICOMPLETE] = s_pattern;
    return ret;
}

bool
KnowledgeBase::acceptable(Rule& src) {
    typedef std::pair<std::multimap<int, Rule>::iterator,
            std::multimap<int, Rule>::iterator> DictionaryRange;

    build_word_index();

    //SentenceDBシーケンス用
    RuleDBType::iterator sent_it;
    bool filted;
    bool is_applied = false;

    sent_it = sentenceDB.begin();
    while (!is_applied && sent_it != sentenceDB.end()) {

        //拡張用:内部言語列長の同一性検査
        // 将来的に内部言語列長が異なるものがデータベースに入るかも知れないので
        if ((*sent_it).internal.size() != src.internal.size()) {
            sent_it++;
            continue;
        }

        //高速化枝狩り
        //対象が1カ所でも一致していないものは使えない
        filted = true;
        for (int index = 0; index < src.internal.size() && filted; index++) {
            if ((*sent_it).internal[index].is_ind()
                    && src.internal[index] != (*sent_it).internal[index]) {
                filted = false;
            }
        }
        if (!filted) {
            sent_it++;
            continue;
        }

        //ある単語規則に対するグラウンドパターン検索
        Element grnd_elm, mean_elm;

        is_applied = true;
        for (int in_idx = 0; is_applied && in_idx < (*sent_it).internal.size();
                in_idx++) {
            grnd_elm = (*sent_it).internal[in_idx]; //検査するインターナル要素
            mean_elm = src.internal[in_idx]; //基準のインターナル要素

            if (grnd_elm == mean_elm) { //単語がそのまま一致する場合
                continue;
            } else if (//変数の場合で、グラウンド可能な場合
                    grnd_elm.is_var() && //変数で
                    word_dic.find(grnd_elm.cat) != word_dic.end() && //変数のカテゴリが辞書に有り
                    word_dic[grnd_elm.cat].find(mean_elm.obj)
                    != word_dic[grnd_elm.cat].end() //辞書の指定カテゴリに単語がある
                    ) {
                continue;
            } else { //構成不可能文規則
                is_applied = false;
            }
        } //内部言語のグラウンドループ

        sent_it++;
    } //文規則のループ

    return is_applied;
}

std::vector<Rule>
KnowledgeBase::grounded_rules(Rule src) {
    RuleDBType grounded_rules;
    std::map<PATTERN_TYPE, std::vector<PatternType> > patterns, patterns2;

    patterns = construct_grounding_patterns(src);
    patterns2 = natural_construct_grounding_patterns(src);
    exception_filter(patterns, patterns2);

    if (patterns[ABSOLUTE].size() == 0 && patterns[COMPLETE].size() == 0)
        return grounded_rules;

    if (patterns[ABSOLUTE].size() != 0) {
        std::vector<PatternType>::iterator it;
        it = patterns[ABSOLUTE].begin();
        while (it != patterns[ABSOLUTE].end()) {
            grounded_rules.push_back((*it).front());
            it++;
        }
    }

    if (patterns[COMPLETE].size() != 0) {
        std::vector<PatternType>::iterator pat_it;
        pat_it = patterns[COMPLETE].begin();

        while (pat_it != patterns[COMPLETE].end()) {
            Rule grounded_rule;
            grounded_rule = src;
            graund_with_pattern(grounded_rule, (*pat_it));
            grounded_rules.push_back(grounded_rule);
            pat_it++;
        }
    }

    return grounded_rules;
}

std::vector<Rule>
KnowledgeBase::grounded_rules2(Rule src, std::vector<KnowledgeBase::PatternType>& all_patterns) {
    RuleDBType grounded_rules;
    std::map<PATTERN_TYPE, std::vector<PatternType> > patterns, natural_patterns;
    std::vector<PatternType>::iterator nat_it;

    patterns = construct_grounding_patterns(src);
    natural_patterns = natural_construct_grounding_patterns(src);
    exception_filter(patterns, natural_patterns);

    if (patterns[ABSOLUTE].size() == 0 && patterns[COMPLETE].size() == 0)
        return grounded_rules;

    if (patterns[ABSOLUTE].size() != 0) {
        std::vector<PatternType>::iterator it;
        it = patterns[ABSOLUTE].begin();
        nat_it = natural_patterns[ABSOLUTE].begin();

        while (it != patterns[ABSOLUTE].end()) {
            grounded_rules.push_back((*it).front());
            all_patterns.push_back(*nat_it);
            nat_it++;
            it++;
        }
    }

    if (patterns[COMPLETE].size() != 0) {
        std::vector<PatternType>::iterator pat_it;
        pat_it = patterns[COMPLETE].begin();
        nat_it = natural_patterns[COMPLETE].begin();

        while (pat_it != patterns[COMPLETE].end()) {
            Rule grounded_rule;
            grounded_rule = src;
            graund_with_pattern(grounded_rule, (*pat_it));
            grounded_rules.push_back(grounded_rule);
            all_patterns.push_back(*nat_it);
            nat_it++;
            pat_it++;
        }
    }

    return grounded_rules;
}

std::vector<Rule>
KnowledgeBase::groundable_rules(Rule& src) {
    build_word_index();

    std::vector<Rule> ret;

    //SentenceDBシーケンス用
    RuleDBType::iterator sent_it;
    bool filted;
    bool is_applied = false;

    sent_it = sentenceDB.begin();
    while (sent_it != sentenceDB.end()) {
        //拡張用:内部言語列長の同一性検査
        // 将来的に内部言語列長が異なるものがデータベースに入るかも知れないので
        if ((*sent_it).internal.size() != src.internal.size()) {
            sent_it++;
            continue;
        }

        //高速化枝狩り
        //対象が1カ所でも一致していないものは使えない
        filted = true;
        for (int index = 0; index < src.internal.size() && filted; index++) {
            if ((*sent_it).internal[index].is_ind()
                    && src.internal[index] != (*sent_it).internal[index]) {
                filted = false;
            }
        }
        if (!filted) {
            sent_it++;
            continue;
        }

        //ある単語規則に対するグラウンドパターン検索
        Element grnd_elm, mean_elm;

        is_applied = true;
        for (int in_idx = 0; is_applied && in_idx < (*sent_it).internal.size();
                in_idx++) {
            grnd_elm = (*sent_it).internal[in_idx]; //検査するインターナル要素
            mean_elm = src.internal[in_idx]; //基準のインターナル要素

            if (grnd_elm == mean_elm) { //単語がそのまま一致する場合
                continue;
            } else if (//変数の場合で、グラウンド可能な場合
                    grnd_elm.is_var() && //変数で
                    word_dic.find(grnd_elm.cat) != word_dic.end() && //変数のカテゴリが辞書に有り
                    word_dic[grnd_elm.cat].find(mean_elm.obj)
                    != word_dic[grnd_elm.cat].end() //辞書の指定カテゴリに単語がある
                    ) {
                continue;
            } else { //構成不可能文規則
                is_applied = false;
            }
        } //内部言語のグラウンドループ

        if (is_applied) {
            ret.push_back(*sent_it);
        }

        sent_it++;
    } //文規則のループ

    return ret;
}

std::string
KnowledgeBase::to_s(void) {
    std::vector<Rule> rule_buf;
    std::vector<std::string> buf;
    std::vector<Rule>::iterator it;
    std::string sbuf;

    rule_buf.clear();
    sbuf = std::string("\nSent BOX\n");
    buf.push_back(sbuf);
    rule_buf = sentence_box;
    //	std::sort(rule_buf.begin(), rule_buf.end(), RuleSort());
    it = rule_buf.begin();
    while (it != rule_buf.end()) {
        buf.push_back((*it).to_s());
        it++;
    }

    rule_buf.clear();
    sbuf = std::string("\nsBOX\n");
    buf.push_back(sbuf);
    rule_buf = sbox_buffer;
    //	std::sort(rule_buf.begin(), rule_buf.end(), RuleSort());
    it = rule_buf.begin();
    while (it != rule_buf.end()) {
        buf.push_back((*it).to_s());
        it++;
    }

    rule_buf.clear();
    sbuf = std::string("\nWord BOX\n");
    buf.push_back(sbuf);
    rule_buf = word_box;
    //	std::sort(rule_buf.begin(), rule_buf.end(), RuleSort());
    it = rule_buf.begin();
    while (it != rule_buf.end()) {
        buf.push_back((*it).to_s());
        it++;
    }

    rule_buf.clear();
    sbuf = std::string("\nSent DB\n");
    buf.push_back(sbuf);
    rule_buf = sentenceDB;
    //	std::sort(rule_buf.begin(), rule_buf.end(), RuleSort());
    it = rule_buf.begin();
    while (it != rule_buf.end()) {
        buf.push_back((*it).to_s());
        it++;
    }

    rule_buf.clear();
    sbuf = std::string("\nWord DB\n");
    buf.push_back(sbuf);
    rule_buf = wordDB;
    //	std::sort(rule_buf.begin(), rule_buf.end(), RuleSort());
    it = rule_buf.begin();
    while (it != rule_buf.end()) {
        buf.push_back((*it).to_s());
        it++;
    }

    return boost::algorithm::join(buf, "\n");
}

void
KnowledgeBase::logging_on(void) {
    LOGGING_FLAG = true;
}

void
KnowledgeBase::logging_off(void) {
    LOGGING_FLAG = false;
}

void
KnowledgeBase::omissionA_on(void) {
    OMISSION_FLAG = true;
    OMISSION_A = true;
}

void
KnowledgeBase::omissionA_off(void) {
    OMISSION_FLAG = false;
    OMISSION_A = false;
}

void
KnowledgeBase::omissionB_on(void) {
    OMISSION_FLAG = true;
    OMISSION_B = true;
}

void
KnowledgeBase::omissionB_off(void) {
    OMISSION_FLAG = false;
    OMISSION_B = false;
}

void
KnowledgeBase::omissionC_on(void) {
    OMISSION_FLAG = true;
    OMISSION_C = true;
}

void
KnowledgeBase::omissionC_off(void) {
    OMISSION_FLAG = false;
    OMISSION_C = false;
}

void
KnowledgeBase::omissionD_on(void) {
    OMISSION_FLAG = true;
    OMISSION_D = true;
}

void
KnowledgeBase::omissionD_off(void) {
    OMISSION_FLAG = false;
    OMISSION_D = false;
}

void
KnowledgeBase::set_control(uint32_t FLAGS) {
    CONTROLS |= FLAGS;
}

Rule
KnowledgeBase::fabricate_idx(Rule& src) {
    if (!indexed) {
        return fabricate(src);
    }

    //vectorize
    std::vector<int> mean_vec;
    for (int j = 0; j < src.internal.size(); j++) {
        mean_vec[j] = src.internal[j].obj;
    }

    //std::cerr << ">>Use Indexing" << std::endl;
    if ((*fabricate_index)[mean_vec][COMPLETE].size() != 0) {
        int i = MT19937::irand() % (*fabricate_index)[mean_vec][COMPLETE].size();
        return (*fabricate_index)[mean_vec][COMPLETE][i];
    }
    //std::cerr << ">>C" << std::endl;
    
    if ((*fabricate_index)[mean_vec][ABSOLUTE].size() != 0) {
        int i = MT19937::irand() % (*fabricate_index)[mean_vec][ABSOLUTE].size();
        return (*fabricate_index)[mean_vec][ABSOLUTE][i];
    }
    //std::cerr << ">>A" << std::endl;

    if ((*fabricate_index)[mean_vec][SEMICOMPLETE].size() != 0) {
        int i = MT19937::irand() % (*fabricate_index)[mean_vec][SEMICOMPLETE].size();
        return (*fabricate_index)[mean_vec][SEMICOMPLETE][i];
    }
    //std::cerr << ">>S" << std::endl;

    return (*fabricate_index)[mean_vec][RANDOM].front();
}

Rule
KnowledgeBase::fabricate_idx_min(Rule& src) {
    if (!indexed)
        return fabricate_min_len(src);

    std::vector<int> mean_vec;
    for (int j = 0; j < src.internal.size(); j++) {
        mean_vec[j] = src.internal[j].obj;
    }

    return (*fabricate_index)[mean_vec][SORTED_ALL].front();
}

void
KnowledgeBase::indexer(std::vector<Rule>& meanings) {
    //std::cerr << ">>In Indexing" << std::endl;
    fabricate_index = boost::shared_ptr<IndexT>(new IndexT());

    for (int i = 0; i < meanings.size(); i++) {
        //get pattern
        std::map<PATTERN_TYPE, std::vector<PatternType> > all_patterns, all_patterns2;
        all_patterns = construct_grounding_patterns(meanings[i]);
        all_patterns2 = natural_construct_grounding_patterns(meanings[i]);
        exception_filter(all_patterns, all_patterns2);

        //std::cerr << "A";
        //vectorize
        std::vector<int> mean_vec;
        for (int j = 0; j < meanings[i].internal.size(); j++) {
            mean_vec.push_back(meanings[i].internal[j].obj);
        }


        (*fabricate_index)[mean_vec] = boost::unordered_map<int, std::vector<Rule> >();
        (*fabricate_index)[mean_vec][ABSOLUTE] = std::vector<Rule>();
        (*fabricate_index)[mean_vec][COMPLETE] = std::vector<Rule>();
        (*fabricate_index)[mean_vec][SEMICOMPLETE] = std::vector<Rule>();
        (*fabricate_index)[mean_vec][RANDOM] = std::vector<Rule>();
        (*fabricate_index)[mean_vec][SORTED_ALL] = std::vector<Rule>();



        //まずAbsoluteをプッシュ
        for (int k = 0; k < all_patterns[ABSOLUTE].size(); k++) {
            (*fabricate_index)[mean_vec][ABSOLUTE].push_back(
                    all_patterns[ABSOLUTE][k].front());
            (*fabricate_index)[mean_vec][SORTED_ALL].push_back(
                    all_patterns[ABSOLUTE][k].front());

        }


        //次にCompleteをグラウンドしてプッシュ
        for (int j = 0; j < all_patterns[COMPLETE].size(); j++) {
            Rule grounded_rule;
            grounded_rule = meanings[i];
            graund_with_pattern(grounded_rule, all_patterns[COMPLETE][j]);
            (*fabricate_index)[mean_vec][COMPLETE].push_back(grounded_rule);
            (*fabricate_index)[mean_vec][SORTED_ALL].push_back(grounded_rule);
        }


        //最後にインベンションしてプッシュ
        for (int j = 0; j < all_patterns[SEMICOMPLETE].size(); j++) {
            //completion
            PatternType use_pattern;
            PatternType::iterator it;

            use_pattern = all_patterns[SEMICOMPLETE][j];

            for (it = use_pattern.begin(); it != use_pattern.end(); it++) {
                if ((*it).is_noun() && (*it).external.size() == 0) {
                    ExType buzz;
                    buzz = construct_buzz_word();
                    (*it).external = buzz;
                    break;
                }
            }

            Rule grounded_rule;
            grounded_rule = meanings[i];
            graund_with_pattern(grounded_rule, use_pattern);
            (*fabricate_index)[mean_vec][SEMICOMPLETE].push_back(grounded_rule);
            (*fabricate_index)[mean_vec][SORTED_ALL].push_back(grounded_rule);
        }

        //最後にもし発話できないときランダムをプッシュ
        if ((*fabricate_index)[mean_vec][ABSOLUTE].size() == 0
                && (*fabricate_index)[mean_vec][COMPLETE].size() == 0
                && (*fabricate_index)[mean_vec][SEMICOMPLETE].size() == 0) {
            ExType ex;
            ex = construct_buzz_word();

            Rule grounded_rule;
            grounded_rule = meanings[i];

            grounded_rule.external = ex;
            (*fabricate_index)[mean_vec][RANDOM].push_back(grounded_rule);
            (*fabricate_index)[mean_vec][SORTED_ALL].push_back(grounded_rule);
        }

        std::sort(
                (*fabricate_index)[mean_vec][SORTED_ALL].begin(),
                (*fabricate_index)[mean_vec][SORTED_ALL].end(),
                ExternalSizeSort()
                );

        //Rule minlen = (*fabricate_index)[mean_vec][SORTED_ALL].front();
        //(*fabricate_index)[mean_vec][SORTED_ALL].clear();
        //std::vector<Rule>((*fabricate_index)[mean_vec][SORTED_ALL]).swap((*fabricate_index)[mean_vec][SORTED_ALL]);
        //(*fabricate_index)[mean_vec][SORTED_ALL].push_back(minlen);
    }

    indexed = true;
    //std::cerr << "<<Out Indexing" << std::endl;
}

void
KnowledgeBase::exception_filter(std::map<KnowledgeBase::PATTERN_TYPE, std::vector<KnowledgeBase::PatternType> >& target_all_patterns, std::map<KnowledgeBase::PATTERN_TYPE, std::vector<KnowledgeBase::PatternType> >& base_all_patterns) {
    if (exception.size() > 0) {
        std::vector<std::vector<Rule> >::iterator exception_it = exception.begin();
        std::vector<PatternType>::iterator target_it;
        std::vector<PatternType>::iterator base_it;
        std::vector<PatternType> erase_list;
        std::vector<PatternType>::iterator erase_list_it;
        bool applied_exp;
        for (; exception_it != exception.end(); exception_it++) {

            applied_exp = false;

            target_it = target_all_patterns[COMPLETE].begin();
            base_it = base_all_patterns[COMPLETE].begin();
            while (target_it != target_all_patterns[COMPLETE].end() && base_it != base_all_patterns[COMPLETE].end()&&(!applied_exp)) {

                if ((*base_it) == (*exception_it)) {
                    erase_list.push_back(*target_it);
                    applied_exp = true;
                }

                target_it++;
                base_it++;
            }

            target_it = target_all_patterns[ABSOLUTE].begin();
            base_it = base_all_patterns[ABSOLUTE].begin();
            while (target_it != target_all_patterns[ABSOLUTE].end() && base_it != base_all_patterns[ABSOLUTE].end()&&(!applied_exp)) {

                if ((*base_it) == (*exception_it)) {
                    erase_list.push_back(*target_it);
                    applied_exp = true;
                }

                target_it++;
                base_it++;
            }

            target_it = target_all_patterns[SEMICOMPLETE].begin();
            base_it = base_all_patterns[SEMICOMPLETE].begin();
            while (target_it != target_all_patterns[SEMICOMPLETE].end() && base_it != base_all_patterns[SEMICOMPLETE].end()&&(!applied_exp)) {

                if ((*base_it) == (*exception_it)) {
                    erase_list.push_back(*target_it);
                    applied_exp = true;
                }

                target_it++;
                base_it++;
            }

        }
        erase_list_it = erase_list.begin();
        for (; erase_list_it != erase_list.end(); erase_list_it++) {

            //            std::cerr << "erase rules" << std::endl;
            //std::cerr << erase_list_it << std::endl;

            applied_exp = false;

            for (int i = 0; i < target_all_patterns[COMPLETE].size(); i++) {
                if (target_all_patterns[COMPLETE][i] == (*erase_list_it)) {
                    target_all_patterns[COMPLETE].erase(target_all_patterns[COMPLETE].begin() + i);
                    base_all_patterns[COMPLETE].erase(base_all_patterns[COMPLETE].begin() + i);
                    applied_exp = true;
                    break;
                }
            }

            for (int i = 0; i < target_all_patterns[ABSOLUTE].size(); i++) {
                if (target_all_patterns[ABSOLUTE][i] == (*erase_list_it)) {
                    target_all_patterns[ABSOLUTE].erase(target_all_patterns[ABSOLUTE].begin() + i);
                    base_all_patterns[ABSOLUTE].erase(base_all_patterns[ABSOLUTE].begin() + i);
                    applied_exp = true;
                    break;
                }
            }

            for (int i = 0; i < target_all_patterns[SEMICOMPLETE].size(); i++) {
                if (target_all_patterns[SEMICOMPLETE][i] == (*erase_list_it)) {
                    target_all_patterns[SEMICOMPLETE].erase(target_all_patterns[SEMICOMPLETE].begin() + i);
                    base_all_patterns[SEMICOMPLETE].erase(base_all_patterns[SEMICOMPLETE].begin() + i);
                    applied_exp = true;
                    break;
                }
            }

        }
    }
}

/*bool
KnowledgeBase::omission(Rule& mean, KnowledgeBase::PatternType& ptn, KnowledgeBase::PatternType& res) {
    KnowledgeBase::PatternType src, tmp_ptn;
    Rule new_rule, tmp_rule, test = mean, min_lev_rule;
    bool flag = true, changed = false, loop_flag = true;
    int index = MT19937::irand(), size = ptn.size(), count = 0, add;
    std::vector<int> check, omitted_log;
    std::vector<Rule> kb_all, lev_rules;
    std::vector<Rule>::iterator kb_all_it, lev_rules_it;
    std::vector<Element> copy_EL, tmp_ev;
    std::vector<Element>::iterator copy_EL_it;
    double lev_tmp, min_lev;
    std::vector<std::vector<Element> > edited_ev;

    //        std::cout << "OMISSION" << std::endl;
    //std::cout << "UTTERANCES COUNT : " << kb_all.size() << std::endl;
    src = ptn;
    //文字を抜く作業
    if (OMISSION_A) { //抜けるだけ抜いてから次へ.レーベンシュタイン距離でランク付け
        /*kb_all = utterances();
        //        std::cout << "IN1" << std::endl;
        if (size > 1) { //単語ルールがあれば
            while ((count < (size - 1))) {// && flag) {
                //                std::cout << "IN2" << std::endl;
                //インデックスを決める作業
                do {
                    index = index % (size - 1) + 1;
                } while ((std::find(check.begin(), check.end(), index) != check.end()));
                //std::cout << "IN2.1" << std::endl;
                check.push_back(index); //選ばれたインデックスを記憶
                //                std::cout << src[index].to_s() << std::endl;
                new_rule = src[index];
                while (flag && omission_check(src[index])) {
                    //                    std::cout << "IN3" << std::endl;
                    //文字を抜く作業tmp_ruleに入れる. そのあと，srcに入れてground_with_patternで構築してtestに"省略"の一時的結果を返す
                    //tmp_rule = src[index];
                    //std::cout << tmp_rule.to_s() << std::endl;
                    //                    for (int iii = 0; iii < src.size(); iii++) {
                    //                        std::cout << src[iii].to_s() << std::endl;
                    //                    }
                    //if (tmp_rule.external.size() % 2 == 1)
                    //    tmp_rule.external.erase(tmp_rule.external.begin() + (tmp_rule.external.size() - 1) / 2);
                    //else {
                    //    add = MT19937::irand() % 2; //真ん中より下か上かランダムに決定
                    //    tmp_rule.external.erase(tmp_rule.external.begin() + (tmp_rule.external.size() / 2 - add));
                    //}
                    edited_ev = cut_out_letter(src[index].external);
                    //std::cout << tmp_rule.to_s() << std::endl;
                    src[index].external = edited_ev[0];
                    //                    for (int iii = 0; iii < src.size(); iii++) {
                    //                        std::cout << src[iii].to_s() << std::endl;
                    //                    }
                    tmp_ptn = src; //graund_with_patternでsrcが破壊されるのを防ぐ
                    graund_with_pattern(test, tmp_ptn);
                    //                    std::cout << src.size() << std::endl;
                    //                    std::cout << test.to_s() << std::endl;
                    //順位が一位か調べる作業flagに結果を返す
                    min_lev_rule = test; //lev_rulesの中からひとつだけ選ぶ場合に使うけど，今は使わない
                    min_lev = lev_tmp = Distance::levenstein2(test.external, (*kb_all.begin()).external);
                    kb_all_it = kb_all.begin();
                    for (; kb_all_it != kb_all.end(); kb_all_it++) {
                        lev_tmp = Distance::levenstein2(test.external, (*kb_all_it).external); //testと*kb_all_itとのレーベンシュタイン距離
                        if (min_lev > lev_tmp) {
                            lev_rules.clear();
                            lev_rules.push_back(*kb_all_it);
                        } else if (min_lev == lev_tmp) {
                            lev_rules.push_back(*kb_all_it);
                        }
                    }
                    //                    std::cout << "IN3.1" << std::endl;
                    //lev_rulesの中からmin_lev_ruleを選ぶ.（今回はすべて同じかを調べるため，min_lev_rulesは使わない）
                    flag = true;
                    lev_rules_it = lev_rules.begin();
                    for (; lev_rules_it != lev_rules.end(); lev_rules_it++) {
                        flag &= (test.internal == (*lev_rules_it).internal);
                    }
                    if (lev_rules.size() == 0) {
                        flag = false;
                    }
                    //                    std::cout << "IN3.2" << std::endl;
                    if (flag) {
                        new_rule = tmp_rule;
                        changed = true;
                    }
                    src[index] = new_rule; //srcに入れてたルールをもとに戻す or srcに新しいルールを入れる
                    //                    std::cout << "IN3.3" << std::endl;
                }
                //                std::cout << "IN2.1" << std::endl;
                //                for (int iii = 0; iii < src.size(); iii++) {
                //                    std::cout << src[iii].to_s() << std::endl;
                //                }
                //                std::cout << src[index].to_s() << std::endl;
                flag = true; //全ルールでomissionを行う
                count += 1;
            }
        }
        //文ルールの省略処理
        if (flag) {
            //            std::cout << "Initial rule " << src[0].to_s() << std::endl;
            int location = 0, now_loc = 0;
            tmp_rule = src[0];
            new_rule = src[0];
            for (int i = 0; i < size; i++) {
                omitted_log.push_back(0);
            }
            while (location < size) {
                flag = true;
                //                std::cout << "Base rule " << new_rule.to_s() << std::endl;
                while (flag) {
                    now_loc = 0;
                    loop_flag = true;
                    copy_EL = tmp_rule.external;
                    new_rule.external.clear();
                    tmp_ev.clear();
                    copy_EL_it = copy_EL.begin();
                    for (; copy_EL_it != copy_EL.end(); copy_EL_it++) {
                        //文字を抜く作業
                        //最後の文字が終端記号ならtmp_evに追加
                        if ((copy_EL_it == copy_EL.end() - 1)&&(*copy_EL_it).is_sym())
                            tmp_ev.push_back(*copy_EL_it);
                        //区切り文字はnullかカテゴリ
                        if ((*copy_EL_it).is_cat() || (copy_EL_it == copy_EL.end() - 1)) {
                            now_loc += 1;
                            if (tmp_ev.size() != 0 && tmp_ev.size() > 2 && omitted_log[now_loc] != 1) {
                                loop_flag = false;
                                //                                if (tmp_ev.size() % 2 == 1)
                                //                                    tmp_ev.erase(tmp_ev.begin() + (tmp_ev.size() - 1) / 2);
                                //                                else {
                                //                                    add = MT19937::irand() % 2; //真ん中より下か上かランダムに決定
                                //                                    tmp_ev.erase(tmp_ev.begin() + (tmp_ev.size() / 2 - add));
                                //                                }
                                edited_ev = cut_out_letter(tmp_ev);
                                tmp_ev = edited_ev[0];
                            } else if ((copy_EL_it == (copy_EL.end() - 1))&&(*(copy_EL_it)).is_cat()) { //文字省略出来なくて最後の文字列だった場合（最後の文字列がnullだった場合は前の文字列が省略できなかったらサーチしてる場所を次へ移す．これは文ルールの文字省略の終了と同義）
                                now_loc += 1;
                            }
                            if (tmp_ev.size() != 0)
                                new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                            tmp_ev.clear();
                            if ((!loop_flag)&&(*copy_EL_it).is_cat()) {
                                new_rule.external.insert(new_rule.external.end(), copy_EL_it, copy_EL.end());
                                break;
                            } else if ((!loop_flag)) {
                                break;
                            } else if ((*copy_EL_it).is_cat()) {
                                new_rule.external.push_back(*copy_EL_it);
                            }
                        } else {
                            tmp_ev.push_back(*copy_EL_it);
                        }
                    }
                    //                if(tmp_ev.size()!=0){
                    //                    new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                    //                    tmp_ev.clear();
                    //                }
                    //                    std::cout << "Next rule " << new_rule.to_s() << std::endl;
                    //                    std::cout << "TEST " << now_loc << std::endl;
                    if (loop_flag) {
                        flag = false;
                    } else {
                        //一位かどうか調べる
                        src[0] = new_rule;
                        graund_with_pattern(test, src); //ground_with_patternでsrcが破壊されないようにしたためsrcを直接使っても良い
                        min_lev_rule = test; //lev_rulesの中からひとつだけ選ぶ場合に使うけど，今は使わない
                        min_lev = Distance::levenstein2(test.external, (*kb_all.begin()).external);
                        kb_all_it = kb_all.begin();
                        for (; kb_all_it != kb_all.end(); kb_all_it++) {
                            lev_tmp = Distance::levenstein2(test.external, (*kb_all_it).external); //testと*kb_all_itとのレーベンシュタイン距離
                            if (min_lev > lev_tmp) {
                                lev_rules.clear();
                                lev_rules.push_back(*kb_all_it);
                            } else if (min_lev == lev_tmp) {
                                lev_rules.push_back(*kb_all_it);
                            }
                        }
                        //                    std::cout << "IN3.1" << std::endl;
                        //lev_rulesの中からmin_lev_ruleを選ぶ.（今回はすべて同じかを調べるため，min_lev_rulesは使わない）
                        flag = true;
                        lev_rules_it = lev_rules.begin();
                        for (; lev_rules_it != lev_rules.end(); lev_rules_it++) {
                            flag &= (test.internal == (*lev_rules_it).internal);
                        }
                        if (lev_rules.size() == 0) {
                            flag = false;
                        }
                        //
                        if (flag) {
                            tmp_rule = new_rule;
                        } else {
                            src[0] = tmp_rule;
                            new_rule = tmp_rule;
                            omitted_log[now_loc] = 1; //これ以上省略できないという印
                        }
                    }
                }
                location = now_loc;
            }

        }
//
    } else if (OMISSION_B) {//省略処理をしたものを組み込んだ発話が, 他の発話の中でreplaceableなら, 省略できない
        /*       kb_all = utterances();
               bool replaceable, ematched, replaceable1, replaceable2;
               int add2;
               Rule tmp_rule2, new_rule2;
               std::vector<Element> tmp_ev2;
               //                std::cout << "IN1" << std::endl;
               if (size > 1) { //単語ルールがあれば
                   while ((count < (size - 1))) {// && flag) {
                       //                                std::cout << "IN2" << std::endl;
                       //インデックスを決める作業
                       do {
                           index = index % (size - 1) + 1;
                       } while ((std::find(check.begin(), check.end(), index) != check.end()));
                       //                std::cout << "IN2.1" << std::endl;
                       check.push_back(index); //選ばれたインデックスを記憶
                       //                std::cout << src[index].to_s() << std::endl;
                       new_rule = src[index];
                       while (flag && omission_check(src[index])) {
                           //                    std::cout << "IN3" << std::endl;
                           //文字を抜く作業tmp_ruleに入れる. そのあと，srcに入れてground_with_patternで構築してtestに"省略"の一時的結果を返す
                           tmp_rule = src[index];
                           //std::cout << tmp_rule.to_s() << std::endl;
                           //                    for (int iii = 0; iii < src.size(); iii++) {
                           //                        std::cout << src[iii].to_s() << std::endl;
                           //                    }

                           edited_ev = cut_out_letter(tmp_rule.external);
                           if (edited_ev.size() == 1) {
                               tmp_rule.external = edited_ev[0];
                               src[index] = tmp_rule;
                               tmp_ptn = src; //graund_with_patternでsrcが破壊されるのを防ぐ
                               graund_with_pattern(test, tmp_ptn);
                               //全発話文字列の中に文字省略後の文字列があればfalse, なければtrueをflagに代入
                               replaceable = false;
                               kb_all_it = kb_all.begin();
                               while (kb_all_it != kb_all.end()) {
                                   //ガード
                                   if ((*kb_all_it).external.size() < test.external.size()) {
                                       kb_all_it++;
                                       continue; //while
                                   }
                                   int ex_limit;
                                   ex_limit = (*kb_all_it).external.size() - test.external.size();
                                   ematched = false;
                                   for (int i = 0; !ematched && i <= ex_limit; i++) {
                                       if (std::equal((*kb_all_it).external.begin() + i,
                                               (*kb_all_it).external.begin() + i + test.external.size(),
                                               test.external.begin())) {
                                           ematched = true;
                                           break;
                                       }
                                   }
                                   //後処理
                                   if (!ematched) { //外部言語列に一致部分無し
                                       kb_all_it++;
                                       continue; // while
                                   } else {
                                       replaceable = true;
                                       break;
                                   }
                               }
                           } else {
                               tmp_rule2 = tmp_rule;
                               tmp_rule.external = edited_ev[0];
                               tmp_rule2.external = edited_ev[1];
                               src[index] = tmp_rule;
                               tmp_ptn = src; //graund_with_patternでsrcが破壊されるのを防ぐ
                               graund_with_pattern(test, tmp_ptn);
                               //全発話文字列の中に文字省略後の文字列があればfalse, なければtrueをflagに代入
                               replaceable1 = false;
                               kb_all_it = kb_all.begin();
                               while (kb_all_it != kb_all.end()) {
                                   //ガード
                                   if ((*kb_all_it).external.size() < test.external.size()) {
                                       kb_all_it++;
                                       continue; //while
                                   }
                                   int ex_limit;
                                   ex_limit = (*kb_all_it).external.size() - test.external.size();
                                   ematched = false;
                                   for (int i = 0; !ematched && i <= ex_limit; i++) {
                                       if (std::equal((*kb_all_it).external.begin() + i,
                                               (*kb_all_it).external.begin() + i + test.external.size(),
                                               test.external.begin())) {
                                           ematched = true;
                                           break;
                                       }
                                   }
                                   //後処理
                                   if (!ematched) { //外部言語列に一致部分無し
                                       kb_all_it++;
                                       continue; // while
                                   } else {
                                       replaceable1 = true;
                                       break;
                                   }
                               }
                               src[index] = tmp_rule2;
                               tmp_ptn = src; //graund_with_patternでsrcが破壊されるのを防ぐ
                               graund_with_pattern(test, tmp_ptn);
                               //全発話文字列の中に文字省略後の文字列があればfalse, なければtrueをflagに代入
                               replaceable2 = false;
                               kb_all_it = kb_all.begin();
                               while (kb_all_it != kb_all.end()) {
                                   //ガード
                                   if ((*kb_all_it).external.size() < test.external.size()) {
                                       kb_all_it++;
                                       continue; //while
                                   }
                                   int ex_limit;
                                   ex_limit = (*kb_all_it).external.size() - test.external.size();
                                   ematched = false;
                                   for (int i = 0; !ematched && i <= ex_limit; i++) {
                                       if (std::equal((*kb_all_it).external.begin() + i,
                                               (*kb_all_it).external.begin() + i + test.external.size(),
                                               test.external.begin())) {
                                           ematched = true;
                                           break;
                                       }
                                   }
                                   //後処理
                                   if (!ematched) { //外部言語列に一致部分無し
                                       kb_all_it++;
                                       continue; // while
                                   } else {
                                       replaceable2 = true;
                                       break;
                                   }
                               }
                               replaceable = (replaceable1 || replaceable2);
                           }
                           flag = (!replaceable);
                           //                                        std::cout << "IN3.2" << std::endl;
                           if (flag) {
                               new_rule = tmp_rule;
                               changed = true;
                           }
                           src[index] = new_rule; //srcに入れてたルールをもとに戻す or srcに新しいルールを入れる
                           //                                        std::cout << "IN3.3" << std::endl;
                       }
                       //                std::cout << "IN2.1" << std::endl;
                       //                for (int iii = 0; iii < src.size(); iii++) {
                       //                    std::cout << src[iii].to_s() << std::endl;
                       //                }
                       //                std::cout << src[index].to_s() << std::endl;
                       flag = true; //全ルールでomissionを行う
                       count += 1;
                   }
               }
               //文ルールの省略処理
               if (flag) {
                   //            std::cout << "SENTENCE" << std::endl;
                   //            std::cout << "Initial rule " << src[0].to_s() << std::endl;
                   int location = 0, now_loc = 0;
                   tmp_rule = src[0];
                   new_rule = src[0];
                   for (int i = 0; i < size; i++) {
                       omitted_log.push_back(0);
                   }
                   while (location < size) {
                       flag = true;
                       //                std::cout << "Base rule " << new_rule.to_s() << std::endl;
                       while (flag) {
                           //                    std::cout << "TEST 7" << std::endl;
                           now_loc = 0;
                           loop_flag = true;
                           copy_EL = tmp_rule.external;
                           new_rule.external.clear();
                           new_rule2.external.clear();
                           tmp_ev.clear();
                           copy_EL_it = copy_EL.begin();
                           for (; copy_EL_it != copy_EL.end(); copy_EL_it++) {
                               //                        std::cout << "TEST 8" << std::endl;
                               //文字を抜く作業
                               //最後の文字が終端記号ならtmp_evに追加
                               if ((copy_EL_it == copy_EL.end() - 1)&&(*copy_EL_it).is_sym()) {
                                   //                            std::cout << "TEST 9" << std::endl;
                                   tmp_ev.push_back(*copy_EL_it);
                               }
                               //区切り文字はnullかカテゴリ
                               if ((*copy_EL_it).is_cat() || (copy_EL_it == copy_EL.end() - 1)) {
                                   //                            std::cout << "TEST 10" << std::endl;
                                   now_loc += 1;
                                   if (tmp_ev.size() != 0) {
                                       if (!(tmp_ev.size() > 2 && omitted_log[now_loc] != 1)) {
                                           //                                    std::cout << "TEST 11" << std::endl;
                                           new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                                       }
                                       if (tmp_ev.size() > 2 && omitted_log[now_loc] != 1) {
                                           //                                    std::cout << "TEST 12" << std::endl;
                                           loop_flag = false;
                                           edited_ev = cut_out_letter(tmp_ev);
                                           if (edited_ev.size() == 1) {
                                               //                                        std::cout << "TEST 15" << std::endl;
                                               tmp_ev = edited_ev[0];
                                               new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                                           } else {
                                               //                                        std::cout << "TEST 16" << std::endl;
                                               tmp_ev2 = tmp_ev;
                                               new_rule2 = new_rule;
                                               tmp_ev = edited_ev[0];
                                               tmp_ev2 = edited_ev[1];
                                               //                                        std::cout << "TEST 19" << std::endl;
                                               new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                                               new_rule2.external.insert(new_rule2.external.end(), tmp_ev2.begin(), tmp_ev2.end());
                                               //                                        std::cout << "TEST 18" << std::endl;
                                           }
                                       } else if ((copy_EL_it == (copy_EL.end() - 1))&&(*(copy_EL_it)).is_cat()) { //文字省略出来なくて最後の文字列だった場合（最後の文字列がnullだった場合は前の文字列が省略できなかったらサーチしてる場所を次へ移す．これは文ルールの文字省略の終了と同義）
                                           now_loc += 1;
                                       }
                                   } else if ((copy_EL_it == (copy_EL.end() - 1))&&(*(copy_EL_it)).is_cat()) {
                                       now_loc += 1;
                                   }
                                   tmp_ev.clear();
                                   //                            std::cout << "TEST 13" << std::endl;
                                   if ((!loop_flag)&&(*copy_EL_it).is_cat()) {
                                       new_rule.external.insert(new_rule.external.end(), copy_EL_it, copy_EL.end());
                                       if (new_rule2.external.size() != 0) {
                                           new_rule2.external.insert(new_rule2.external.end(), copy_EL_it, copy_EL.end());
                                       }
                                       break;
                                   } else if ((!loop_flag)) {
                                       break;
                                   } else if ((*copy_EL_it).is_cat()) {
                                       new_rule.external.push_back(*copy_EL_it);
                                   }
                               } else {
                                   tmp_ev.push_back(*copy_EL_it);
                               }
                               //                        std::cout << "TEST 9" << std::endl;
                           }
                           //                if(tmp_ev.size()!=0){
                           //                    new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                           //                    tmp_ev.clear();
                           //                }
                           //                    std::cout << "Next rule " << new_rule.to_s() << std::endl;
                           //                    std::cout << "TEST " << now_loc << std::endl;
                           if (loop_flag) {
                               flag = false;
                           } else {
                               //省略が認められるかどうか判定(flagの中身を決める)
                               if (new_rule2.external.size() == 0) {
                                   //                            std::cout << "TEST 1" << std::endl;
                                   src[0] = new_rule;
                                   tmp_ptn = src; //graund_with_patternでsrcが破壊されるのを防ぐ
                                   graund_with_pattern(test, tmp_ptn);
                                   //全発話文字列の中に文字省略後の文字列があればfalse, なければtrueをflagに代入
                                   replaceable = false;
                                   kb_all_it = kb_all.begin();
                                   while (kb_all_it != kb_all.end()) {
                                       //ガード
                                       if ((*kb_all_it).external.size() < test.external.size()) {
                                           kb_all_it++;
                                           continue; //while
                                       }
                                       int ex_limit;
                                       ex_limit = (*kb_all_it).external.size() - test.external.size();
                                       ematched = false;
                                       for (int i = 0; !ematched && i <= ex_limit; i++) {
                                           if (std::equal((*kb_all_it).external.begin() + i,
                                                   (*kb_all_it).external.begin() + i + test.external.size(),
                                                   test.external.begin())) {
                                               ematched = true;
                                               break;
                                           }
                                       }
                                       //後処理
                                       if (!ematched) { //外部言語列に一致部分無し
                                           kb_all_it++;
                                           continue; // while
                                       } else {
                                           replaceable = true;
                                           break;
                                       }
                                   }
                                   //                            std::cout << "TEST 3" << std::endl;
                               } else {
                                   //                            std::cout << "TEST 2" << std::endl;
                                   src[0] = new_rule;
                                   tmp_ptn = src; //graund_with_patternでsrcが破壊されるのを防ぐ
                                   graund_with_pattern(test, tmp_ptn);
                                   //全発話文字列の中に文字省略後の文字列があればfalse, なければtrueをflagに代入
                                   replaceable1 = false;
                                   kb_all_it = kb_all.begin();
                                   while (kb_all_it != kb_all.end()) {
                                       //ガード
                                       if ((*kb_all_it).external.size() < test.external.size()) {
                                           kb_all_it++;
                                           continue; //while
                                       }
                                       int ex_limit;
                                       ex_limit = (*kb_all_it).external.size() - test.external.size();
                                       ematched = false;
                                       for (int i = 0; !ematched && i <= ex_limit; i++) {
                                           if (std::equal((*kb_all_it).external.begin() + i,
                                                   (*kb_all_it).external.begin() + i + test.external.size(),
                                                   test.external.begin())) {
                                               ematched = true;
                                               break;
                                           }
                                       }
                                       //後処理
                                       if (!ematched) { //外部言語列に一致部分無し
                                           kb_all_it++;
                                           continue; // while
                                       } else {
                                           replaceable1 = true;
                                           break;
                                       }
                                   }
                                   src[0] = new_rule2;
                                   tmp_ptn = src; //graund_with_patternでsrcが破壊されるのを防ぐ
                                   graund_with_pattern(test, tmp_ptn);
                                   //全発話文字列の中に文字省略後の文字列があればfalse, なければtrueをflagに代入
                                   replaceable2 = false;
                                   kb_all_it = kb_all.begin();
                                   while (kb_all_it != kb_all.end()) {
                                       //ガード
                                       if ((*kb_all_it).external.size() < test.external.size()) {
                                           kb_all_it++;
                                           continue; //while
                                       }
                                       int ex_limit;
                                       ex_limit = (*kb_all_it).external.size() - test.external.size();
                                       ematched = false;
                                       for (int i = 0; !ematched && i <= ex_limit; i++) {
                                           if (std::equal((*kb_all_it).external.begin() + i,
                                                   (*kb_all_it).external.begin() + i + test.external.size(),
                                                   test.external.begin())) {
                                               ematched = true;
                                               break;
                                           }
                                       }
                                       //後処理
                                       if (!ematched) { //外部言語列に一致部分無し
                                           kb_all_it++;
                                           continue; // while
                                       } else {
                                           replaceable2 = true;
                                           break;
                                       }
                                   }
                                   replaceable = (replaceable1 || replaceable2);
                               }
                               flag = (!replaceable);
                               //                        std::cout << "TEST 5" << std::endl;
                               //
                               if (flag) {
                                   tmp_rule = new_rule;
                               } else {
                                   src[0] = tmp_rule;
                                   new_rule = tmp_rule;
                                   omitted_log[now_loc] = 1; //これ以上省略できないという印
                               }
                               //                        std::cout << "TEST 6" << std::endl;
                           }
                       }
                       location = now_loc;
                   }
                   //            std::cout << "SENTENCE FIN." << std::endl;

               }/
    } else if (OMISSION_C) {//単語を省略する．文ルールでは，単語を探す．知識にある全ルールにおいて，replaceableな文字列があれば省略できない．
        //        std::cout << "OMISSION2" << std::endl;
        //単語だけ省略
        kb_all = rules();
        bool replaceable, ematched, replaceable1, replaceable2;
        int add2, target_index;
        Rule tmp_rule2, new_rule2, base_rule;
        std::vector<Element> tmp_ev2;
        //                std::cout << "IN1" << std::endl;
        if (size > 1) { //単語ルールがあれば
            //            std::cout << "OMISSION3" << std::endl;
            while ((count < (size - 1))) {// && flag) {
                //                                                std::cout << "IN2" << std::endl;
                //インデックスを決める作業
                //do {
                //    index = index % (size - 1) + 1;
                //} while ((std::find(check.begin(), check.end(), index) != check.end()));

                //                                std::cout << "IN2.2" << std::endl;
                //check.push_back(index); //選ばれたインデックスを記憶
                //                std::cout << src[index].to_s() << std::endl;
                index = count + 1;
                new_rule = src[index];
                base_rule = src[index];
                //target_indexを求める
                add = 0; //カウンタとしてaddを利用
                for (int mean_ind = 0; mean_ind < mean.internal.size(); mean_ind++) {
                    if (src[0].internal[mean_ind].is_var()) {
                        add += 1;
                        if (index == add) {
                            target_index = mean_ind;
                            break;
                        }
                    }
                }
                while (flag && omission_check(src[index])) {
                    //                    std::cout << "IN3" << std::endl;
                    tmp_rule = src[index];
                    edited_ev = cut_out_letter(tmp_rule.external);
                    if (edited_ev.size() == 1) {
                        //                        std::cout << "IN3.2" << std::endl;
                        tmp_rule.external = edited_ev[0];
                        replaceable = false;
                        kb_all_it = kb_all.begin();
                        while (kb_all_it != kb_all.end()) {
                            //                            std::cout << "IN3.5" << std::endl;
                            //ガード
                            if ((*kb_all_it).external.size() < tmp_rule.external.size()) {
                                kb_all_it++;
                                continue; //while
                            }
                            //                            std::cout << "IN3.6" << std::endl;
                            //                            std::cout << base_rule.to_s() << std::endl;
                            //                            std::cout << (*kb_all_it).to_s() << std::endl;
                            //                            std::cout << mean.internal[target_index].to_s() << std::endl;
                            if ((*kb_all_it).is_noun() && base_rule.external == (*kb_all_it).external && (*kb_all_it).internal[0] == mean.internal[target_index]) {
                                kb_all_it++;
                                continue; //while
                            }
                            //                            std::cout << "IN3.7" << std::endl;
                            int ex_limit;
                            ex_limit = (*kb_all_it).external.size() - tmp_rule.external.size();
                            ematched = false;
                            for (int i = 0; !ematched && i <= ex_limit; i++) {
                                if (std::equal((*kb_all_it).external.begin() + i,
                                        (*kb_all_it).external.begin() + i + tmp_rule.external.size(),
                                        tmp_rule.external.begin())) {
                                    ematched = true;
                                    break;
                                }
                            }
                            //                            std::cout << "IN3.8" << std::endl;
                            //後処理
                            if (!ematched) { //外部言語列に一致部分無し
                                kb_all_it++;
                                continue; // while
                            } else {
                                replaceable = true;
                                break;
                            }
                        }
                    } else {
                        //                        std::cout << "IN3.3" << std::endl;
                        tmp_rule.external = edited_ev[0];
                        tmp_rule2.external = edited_ev[1];
                        kb_all_it = kb_all.begin();
                        while (kb_all_it != kb_all.end()) {
                            //ガード
                            if ((*kb_all_it).external.size() < tmp_rule.external.size()) {
                                kb_all_it++;
                                continue; //while
                            }
                            if ((*kb_all_it).is_noun() && base_rule.external == (*kb_all_it).external && (*kb_all_it).internal[0] == mean.internal[target_index]) {
                                kb_all_it++;
                                continue; //while
                            }
                            int ex_limit;
                            ex_limit = (*kb_all_it).external.size() - tmp_rule.external.size();
                            ematched = false;
                            for (int i = 0; !ematched && i <= ex_limit; i++) {
                                if (std::equal((*kb_all_it).external.begin() + i,
                                        (*kb_all_it).external.begin() + i + tmp_rule.external.size(),
                                        tmp_rule.external.begin())) {
                                    ematched = true;
                                    break;
                                }
                            }
                            //後処理
                            if (!ematched) { //外部言語列に一致部分無し
                                kb_all_it++;
                                continue; // while
                            } else {
                                replaceable1 = true;
                                break;
                            }
                        }
                        kb_all_it = kb_all.begin();
                        while (kb_all_it != kb_all.end()) {
                            //ガード
                            if ((*kb_all_it).external.size() < tmp_rule2.external.size()) {
                                kb_all_it++;
                                continue; //while
                            }
                            if ((*kb_all_it).is_noun() && base_rule.external == (*kb_all_it).external && (*kb_all_it).internal[0] == mean.internal[target_index]) {
                                kb_all_it++;
                                continue; //while
                            }
                            int ex_limit;
                            ex_limit = (*kb_all_it).external.size() - tmp_rule2.external.size();
                            ematched = false;
                            for (int i = 0; !ematched && i <= ex_limit; i++) {
                                if (std::equal((*kb_all_it).external.begin() + i,
                                        (*kb_all_it).external.begin() + i + tmp_rule2.external.size(),
                                        tmp_rule2.external.begin())) {
                                    ematched = true;
                                    break;
                                }
                            }
                            //後処理
                            if (!ematched) { //外部言語列に一致部分無し
                                kb_all_it++;
                                continue; // while
                            } else {
                                replaceable2 = true;
                                break;
                            }
                        }
                        replaceable = (replaceable1 || replaceable2);
                    }
                    //                    std::cout << "IN3.4" << std::endl;
                    flag = (!replaceable);
                    //                                        std::cout << "IN3.2" << std::endl;
                    if (flag) {
                        new_rule = tmp_rule;
                        changed = true;
                    }
                    src[index] = new_rule; //srcに入れてたルールをもとに戻す or srcに新しいルールを入れる
                    //                                                            std::cout << "IN3.1" << std::endl;
                }
                //                                std::cout << "IN2.1" << std::endl;
                //                for (int iii = 0; iii < src.size(); iii++) {
                //                    std::cout << src[iii].to_s() << std::endl;
                //                }
                //                std::cout << src[index].to_s() << std::endl;
                flag = true; //全ルールでomissionを行う
                count += 1;
            }
        }

        //文ルールの省略処理
        if (flag) {
            //            std::cout << "OMISSION4" << std::endl;
            //            std::cout << "SENTENCE" << std::endl;
            //            std::cout << "Initial rule " << src[0].to_s() << std::endl;
            int location = 0, now_loc = 0;
            tmp_rule = src[0];
            new_rule = src[0];
            base_rule = src[0];
            for (int i = 0; i < size; i++) {
                omitted_log.push_back(0);
            }
            while (location < size) {
                flag = true;
                //                                std::cout << "Base rule " << new_rule.to_s() << std::endl;
                while (flag) {
                    //                                        std::cout << "TEST 7" << std::endl;
                    now_loc = 0;
                    loop_flag = true;
                    copy_EL = tmp_rule.external;
                    new_rule.external.clear();
                    new_rule2.external.clear();
                    tmp_ev.clear();
                    copy_EL_it = copy_EL.begin();
                    for (; copy_EL_it != copy_EL.end(); copy_EL_it++) {
                        //                                                std::cout << "TEST 8" << std::endl;
                        //文字を抜く作業
                        //最後の文字が終端記号ならtmp_evに追加
                        if ((copy_EL_it == copy_EL.end() - 1)&&(*copy_EL_it).is_sym()) {
                            //                            std::cout << "TEST 9" << std::endl;
                            tmp_ev.push_back(*copy_EL_it);
                        }
                        //区切り文字はnullかカテゴリ
                        if ((*copy_EL_it).is_cat() || (copy_EL_it == copy_EL.end() - 1)) {
                            //                            std::cout << "TEST 10" << std::endl;
                            now_loc += 1;
                            if (tmp_ev.size() != 0) {
                                if (!(tmp_ev.size() > 2 && omitted_log[now_loc] != 1)) {
                                    //                                    std::cout << "TEST 11" << std::endl;
                                    new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                                }
                                if (tmp_ev.size() > 2 && omitted_log[now_loc] != 1) {
                                    //                                    std::cout << "TEST 12" << std::endl;
                                    loop_flag = false;
                                    edited_ev = cut_out_letter(tmp_ev);
                                    if (edited_ev.size() == 1) {
                                        //                                        std::cout << "TEST 15" << std::endl;
                                        tmp_ev = edited_ev[0];
                                        new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                                    } else {
                                        //                                        std::cout << "TEST 16" << std::endl;
                                        tmp_ev2 = tmp_ev;
                                        new_rule2 = new_rule;
                                        tmp_ev = edited_ev[0];
                                        tmp_ev2 = edited_ev[1];
                                        //                                        std::cout << "TEST 19" << std::endl;
                                        new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                                        new_rule2.external.insert(new_rule2.external.end(), tmp_ev2.begin(), tmp_ev2.end());
                                        //                                        std::cout << "TEST 18" << std::endl;
                                    }
                                } else if ((copy_EL_it == (copy_EL.end() - 1))&&(*(copy_EL_it)).is_cat()) { //文字省略出来なくて最後の文字列だった場合（最後の文字列がnullだった場合は前の文字列が省略できなかったらサーチしてる場所を次へ移す．これは文ルールの文字省略の終了と同義）
                                    now_loc += 1;
                                }
                            } else if ((copy_EL_it == (copy_EL.end() - 1))&&(*(copy_EL_it)).is_cat()) {
                                now_loc += 1;
                            }
                            tmp_ev.clear();
                            //                            std::cout << "TEST 13" << std::endl;
                            if ((!loop_flag)&&(*copy_EL_it).is_cat()) {
                                new_rule.external.insert(new_rule.external.end(), copy_EL_it, copy_EL.end());
                                if (new_rule2.external.size() != 0) {
                                    new_rule2.external.insert(new_rule2.external.end(), copy_EL_it, copy_EL.end());
                                }
                                break;
                            } else if ((!loop_flag)) {
                                break;
                            } else if ((*copy_EL_it).is_cat()) {
                                new_rule.external.push_back(*copy_EL_it);
                            }
                        } else {
                            tmp_ev.push_back(*copy_EL_it);
                        }
                        //                                                std::cout << "TEST 9" << std::endl;
                    }
                    //                if(tmp_ev.size()!=0){
                    //                    new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                    //                    tmp_ev.clear();
                    //                }
                    //                    std::cout << "Next rule " << new_rule.to_s() << std::endl;
                    //                                        std::cout << "TEST[loc] " << now_loc << std::endl;
                    if (loop_flag) {
                        flag = false;
                    } else {
                        //省略が認められるかどうか判定(flagの中身を決める)
                        if (new_rule2.external.size() == 0) {
                            //                                                        std::cout << "TEST 1" << std::endl;
                            src[0] = new_rule;
                            test = new_rule;
                            //tmp_ptn = src; //graund_with_patternでsrcが破壊されるのを防ぐ
                            //graund_with_pattern(test, tmp_ptn);
                            //全発話文字列の中に文字省略後の文字列があればfalse, なければtrueをflagに代入
                            replaceable = false;
                            kb_all_it = kb_all.begin();
                            while (kb_all_it != kb_all.end()) {
                                //ガード
                                if ((*kb_all_it).external.size() < test.external.size()) {
                                    kb_all_it++;
                                    continue; //while
                                }
                                if (base_rule == (*kb_all_it)) {
                                    kb_all_it++;
                                    continue; //while
                                }
                                int ex_limit;
                                ex_limit = (*kb_all_it).external.size() - test.external.size();
                                ematched = false;
                                for (int i = 0; !ematched && i <= ex_limit; i++) {
                                    if (std::equal((*kb_all_it).external.begin() + i,
                                            (*kb_all_it).external.begin() + i + test.external.size(),
                                            test.external.begin())) {
                                        ematched = true;
                                        break;
                                    }
                                }
                                //後処理
                                if (!ematched) { //外部言語列に一致部分無し
                                    kb_all_it++;
                                    continue; // while
                                } else {
                                    replaceable = true;
                                    break;
                                }
                            }
                            //                                                        std::cout << "TEST 3" << std::endl;
                        } else {
                            //                                                        std::cout << "TEST 2" << std::endl;
                            src[0] = new_rule;
                            test = new_rule;
                            //tmp_ptn = src; //graund_with_patternでsrcが破壊されるのを防ぐ
                            //graund_with_pattern(test, tmp_ptn);
                            //全発話文字列の中に文字省略後の文字列があればfalse, なければtrueをflagに代入
                            replaceable1 = false;
                            kb_all_it = kb_all.begin();
                            while (kb_all_it != kb_all.end()) {
                                //ガード
                                if ((*kb_all_it).external.size() < test.external.size()) {
                                    kb_all_it++;
                                    continue; //while
                                }
                                if (base_rule == (*kb_all_it)) {
                                    kb_all_it++;
                                    continue; //while
                                }
                                int ex_limit;
                                ex_limit = (*kb_all_it).external.size() - test.external.size();
                                ematched = false;
                                for (int i = 0; !ematched && i <= ex_limit; i++) {
                                    if (std::equal((*kb_all_it).external.begin() + i,
                                            (*kb_all_it).external.begin() + i + test.external.size(),
                                            test.external.begin())) {
                                        ematched = true;
                                        break;
                                    }
                                }
                                //後処理
                                if (!ematched) { //外部言語列に一致部分無し
                                    kb_all_it++;
                                    continue; // while
                                } else {
                                    replaceable1 = true;
                                    break;
                                }
                            }
                            src[0] = new_rule2;
                            test = new_rule2;
                            //tmp_ptn = src; //graund_with_patternでsrcが破壊されるのを防ぐ
                            //graund_with_pattern(test, tmp_ptn);
                            //全発話文字列の中に文字省略後の文字列があればfalse, なければtrueをflagに代入
                            replaceable2 = false;
                            kb_all_it = kb_all.begin();
                            while (kb_all_it != kb_all.end()) {
                                //ガード
                                if ((*kb_all_it).external.size() < test.external.size()) {
                                    kb_all_it++;
                                    continue; //while
                                }
                                if (base_rule == (*kb_all_it)) {
                                    kb_all_it++;
                                    continue; //while
                                }
                                int ex_limit;
                                ex_limit = (*kb_all_it).external.size() - test.external.size();
                                ematched = false;
                                for (int i = 0; !ematched && i <= ex_limit; i++) {
                                    if (std::equal((*kb_all_it).external.begin() + i,
                                            (*kb_all_it).external.begin() + i + test.external.size(),
                                            test.external.begin())) {
                                        ematched = true;
                                        break;
                                    }
                                }
                                //後処理
                                if (!ematched) { //外部言語列に一致部分無し
                                    kb_all_it++;
                                    continue; // while
                                } else {
                                    replaceable2 = true;
                                    break;
                                }
                            }
                            replaceable = (replaceable1 || replaceable2);
                        }
                        flag = (!replaceable);
                        //                        std::cout << "TEST 5" << std::endl;
                        //
                        if (flag) {
                            tmp_rule = new_rule;
                        } else {
                            src[0] = tmp_rule;
                            new_rule = tmp_rule;
                            omitted_log[now_loc] = 1; //これ以上省略できないという印
                        }
                        //                                                std::cout << "TEST 6" << std::endl;
                    }
                }
                location = now_loc;
            }
            //                        std::cout << "SENTENCE FIN." << std::endl;

        }

    } else if (OMISSION_D) {
        //        std::cout << "OMISSION2" << std::endl;
                //単語だけ省略
                kb_all = rules();
                bool replaceable, ematched, replaceable1, replaceable2;
                int add2, target_index;
                Rule tmp_rule2, new_rule2, base_rule;
                std::vector<Element> tmp_ev2;
                //                std::cout << "IN1" << std::endl;
                if (size > 1) { //単語ルールがあれば
        //            std::cout << "OMISSION3" << std::endl;
                    while ((count < (size - 1))) {// && flag) {
        //                                                std::cout << "IN2" << std::endl;
                        //インデックスを決める作業
                        do {
                            index = index % (size - 1) + 1;
                        } while ((std::find(check.begin(), check.end(), index) != check.end()));
        //                                std::cout << "IN2.2" << std::endl;
                        check.push_back(index); //選ばれたインデックスを記憶
                        //                std::cout << src[index].to_s() << std::endl;
                        new_rule = src[index];
                        base_rule = src[index];
                        //target_indexを求める
                        add = 0; //カウンタとしてaddを利用
                        for (int mean_ind = 0; mean_ind < mean.internal.size(); mean_ind++) {
                            if (src[0].internal[mean_ind].is_var()) {
                                add += 1;
                                if (index == add) {
                                    target_index = mean_ind;
                                    break;
                                }
                            }
                        }
                        while (flag && omission_check(src[index])) {
        //                    std::cout << "IN3" << std::endl;
                            tmp_rule = src[index];
                            edited_ev = cut_out_letter(tmp_rule.external);
                            tmp_rule.external = edited_ev[0];
                            //                                        std::cout << "IN3.2" << std::endl;
                            if (flag) {
                                new_rule = tmp_rule;
                                changed = true;
                            }
                            src[index] = new_rule; //srcに入れてたルールをもとに戻す or srcに新しいルールを入れる
        //                                                            std::cout << "IN3.1" << std::endl;
                        }
        //                                std::cout << "IN2.1" << std::endl;
                        //                for (int iii = 0; iii < src.size(); iii++) {
                        //                    std::cout << src[iii].to_s() << std::endl;
                        //                }
                        //                std::cout << src[index].to_s() << std::endl;
                        flag = true; //全ルールでomissionを行う
                        count += 1;
                    }
                }

                //文ルールの省略処理
                if (flag) {
        //            std::cout << "OMISSION4" << std::endl;
                    //            std::cout << "SENTENCE" << std::endl;
                    //            std::cout << "Initial rule " << src[0].to_s() << std::endl;
                    int location = 0, now_loc = 0;
                    tmp_rule = src[0];
                    new_rule = src[0];
                    base_rule = src[0];
                    for (int i = 0; i < size; i++) {
                        omitted_log.push_back(0);
                    }
                    while (location < size) {
                        flag = true;
        //                                std::cout << "Base rule " << new_rule.to_s() << std::endl;
                        while (flag) {
        //                                        std::cout << "TEST 7" << std::endl;
                            now_loc = 0;
                            loop_flag = true;
                            copy_EL = tmp_rule.external;
                            new_rule.external.clear();
                            new_rule2.external.clear();
                            tmp_ev.clear();
                            copy_EL_it = copy_EL.begin();
                            for (; copy_EL_it != copy_EL.end(); copy_EL_it++) {
        //                                                std::cout << "TEST 8" << std::endl;
                                //文字を抜く作業
                                //最後の文字が終端記号ならtmp_evに追加
                                if ((copy_EL_it == copy_EL.end() - 1)&&(*copy_EL_it).is_sym()) {
                                    //                            std::cout << "TEST 9" << std::endl;
                                    tmp_ev.push_back(*copy_EL_it);
                                }
                                //区切り文字はnullかカテゴリ
                                if ((*copy_EL_it).is_cat() || (copy_EL_it == copy_EL.end() - 1)) {
                                    //                            std::cout << "TEST 10" << std::endl;
                                    now_loc += 1;
                                    if (tmp_ev.size() != 0) {
                                        if (!(tmp_ev.size() > 2 && omitted_log[now_loc] != 1)) {
                                            //                                    std::cout << "TEST 11" << std::endl;
                                            new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                                        }
                                        if (tmp_ev.size() > 2 && omitted_log[now_loc] != 1) {
                                            //                                    std::cout << "TEST 12" << std::endl;
                                            loop_flag = false;
                                            edited_ev = cut_out_letter(tmp_ev);
                                            if (edited_ev.size() == 1) {
                                                //                                        std::cout << "TEST 15" << std::endl;
                                                tmp_ev = edited_ev[0];
                                                new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                                            } else {
                                                //                                        std::cout << "TEST 16" << std::endl;
                                                tmp_ev2 = tmp_ev;
                                                new_rule2 = new_rule;
                                                tmp_ev = edited_ev[0];
                                                tmp_ev2 = edited_ev[1];
                                                //                                        std::cout << "TEST 19" << std::endl;
                                                new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                                                new_rule2.external.insert(new_rule2.external.end(), tmp_ev2.begin(), tmp_ev2.end());
                                                //                                        std::cout << "TEST 18" << std::endl;
                                            }
                                        } else if ((copy_EL_it == (copy_EL.end() - 1))&&(*(copy_EL_it)).is_cat()) { //文字省略出来なくて最後の文字列だった場合（最後の文字列がnullだった場合は前の文字列が省略できなかったらサーチしてる場所を次へ移す．これは文ルールの文字省略の終了と同義）
                                            now_loc += 1;
                                        }
                                    } else if ((copy_EL_it == (copy_EL.end() - 1))&&(*(copy_EL_it)).is_cat()) {
                                        now_loc += 1;
                                    }
                                    tmp_ev.clear();
                                    //                            std::cout << "TEST 13" << std::endl;
                                    if ((!loop_flag)&&(*copy_EL_it).is_cat()) {
                                        new_rule.external.insert(new_rule.external.end(), copy_EL_it, copy_EL.end());
                                        if (new_rule2.external.size() != 0) {
                                            new_rule2.external.insert(new_rule2.external.end(), copy_EL_it, copy_EL.end());
                                        }
                                        break;
                                    } else if ((!loop_flag)) {
                                        break;
                                    } else if ((*copy_EL_it).is_cat()) {
                                        new_rule.external.push_back(*copy_EL_it);
                                    }
                                } else {
                                    tmp_ev.push_back(*copy_EL_it);
                                }
        //                                                std::cout << "TEST 9" << std::endl;
                            }
                            //                if(tmp_ev.size()!=0){
                            //                    new_rule.external.insert(new_rule.external.end(), tmp_ev.begin(), tmp_ev.end());
                            //                    tmp_ev.clear();
                            //                }
                            //                    std::cout << "Next rule " << new_rule.to_s() << std::endl;
        //                                        std::cout << "TEST[loc] " << now_loc << std::endl;
                            if (loop_flag) {
                                flag = false;
                            } else {
                                src[0] = new_rule;
                                test = new_rule;
                                //省略が認められるかどうか判定(flagの中身を決める)
                         
                                //                        std::cout << "TEST 5" << std::endl;
                                //
                                if (flag) {
                                    tmp_rule = new_rule;
                                } else {
                                    src[0] = tmp_rule;
                                    new_rule = tmp_rule;
                                    omitted_log[now_loc] = 1; //これ以上省略できないという印
                                }
        //                                                std::cout << "TEST 6" << std::endl;
                            }
                        }
                        location = now_loc;
                    }
        //                        std::cout << "SENTENCE FIN." << std::endl;

                }*

    }
    res = src;

    return changed;
}*/

std::vector<Rule>
KnowledgeBase::utterances(void) {
    std::vector<Rule> kb_all, kb_pat;
    std::vector<Rule>::iterator mean_it, kb_pat_it;
    mean_it = KnowledgeBase::MEANING_SPACE.begin();
    for (; mean_it != KnowledgeBase::MEANING_SPACE.end(); mean_it++) {
        kb_pat = grounded_rules(*mean_it);
        if (kb_pat.size() != 0) {
            kb_pat_it = kb_pat.begin();
            for (; kb_pat_it != kb_pat.end(); kb_pat_it++) {
                kb_all.push_back((*kb_pat_it));
            }
        } else {
            //組み立てられない場合（inventionするか否か）
        }
    }
    return kb_all;
}

bool
KnowledgeBase::clipping(Rule& mean, KnowledgeBase::PatternType& ptn, KnowledgeBase::PatternType& res) {
//    std::cerr << "HHHHHHHHHHHH" << std::endl;
    KnowledgeBase::PatternType src;
    Rule new_rule, tmp_rule, base_rule;
    bool changed = false, clipped, replaceable, sym_flag;
    int ptn_index, mean_index, ptn_size = ptn.size(), count, work;
    std::vector<int> clipped_log;
    std::vector<Rule> kb_rules;
    std::vector<Rule>::iterator kb_it;
    std::vector<Element> tmp_string, clipped_string, buf;
    std::vector<Element>::iterator term_it;
    std::vector<std::vector<Element> > term_strings;
    std::vector<std::vector<Element> >::iterator term_strings_it;
    kb_rules = rules();
    src=ptn;

    //単語の省略
    if (ptn_size > 1) { //単語ルールがあれば
        for (count = 0; count < (ptn_size - 1); count++) {

            ptn_index = count + 1;
            if (src[ptn_index].external.size() > 1) {
//                std::cerr << "JJJJJJJJJJJ" << std::endl;
                clipped = false;
                new_rule = src[ptn_index];
                base_rule = src[ptn_index];
                tmp_rule = new_rule;

                //src[ptn_index]の単語ルールが意味のどれを表現する役割を担うのか探る
                work = 0;
                for (mean_index = ptn_index - 1; mean_index < mean.internal.size(); mean_index++) {
                    if (src[0].internal[mean_index].is_var()) {
                        work++;
                        if (ptn_index == work) {
                            break;
                        }
                    }
                }
                do {
                    tmp_rule.external.erase(tmp_rule.external.end() - 1); //最後の文字を消す
                    replaceable = false;
                    kb_it = kb_rules.begin();
                    while (kb_it != kb_rules.end()) {
                        //ガード
                        if ((*kb_it).external.size() < tmp_rule.external.size()) {
                            kb_it++;
                            continue; //while
                        }
                        if ((*kb_it).is_noun() && base_rule.external == (*kb_it).external && (*kb_it).internal[0] == mean.internal[mean_index]) {
                            kb_it++;
                            continue; //while
                        }
                        work = (*kb_it).external.size() - tmp_rule.external.size();
                        replaceable = false;
                        for (int i = 0; (!replaceable) && i <= work; i++) {
                            if (std::equal((*kb_it).external.begin() + i,
                                    (*kb_it).external.begin() + i + tmp_rule.external.size(),
                                    tmp_rule.external.begin())) {
                                replaceable = true;
                                break;
                            }
                        }


                        if (!replaceable) { //外部言語列に一致部分無し
                            kb_it++;
                            continue; // while
                        } else {
                            break;
                        }
                    }
                    if (!replaceable) {
                        if (!clipped) {
                            clipped = true;
                            if (!changed) {
                                changed = true;
                            }
                        }
                        new_rule = tmp_rule;
                    } else {
                        break;
                    }
                } while ((!replaceable) && new_rule.external.size() > 1);

                if (clipped)
                    src[ptn_index] = new_rule;
            }
        }
    }
//std::cerr << "IIIIIIII" << std::endl;
    //文の省略
    term_strings = recognize_terminal_strings(src[0]);
    term_strings_it = term_strings.begin();
    for (; term_strings_it != term_strings.end(); term_strings_it++) {
        if ((*term_strings_it).size() > 1) {
            clipped_string = *term_strings_it;
            tmp_string = clipped_string;
            clipped = false;

            do {
                tmp_string.erase(tmp_string.end() - 1); //最後の文字を消す
                replaceable = false;
                kb_it = kb_rules.begin();
                while (kb_it != kb_rules.end()) {
                    //ガード
                    if ((*kb_it).external.size() < tmp_string.size()) {
                        kb_it++;
                        continue; //while
                    }
                    if (src[0] == (*kb_it)) {
                        kb_it++;
                        continue; //while
                    }
                    work = (*kb_it).external.size() - tmp_string.size();
                    replaceable = false;
                    for (int i = 0; !replaceable && i <= work; i++) {
                        if (std::equal((*kb_it).external.begin() + i,
                                (*kb_it).external.begin() + i + tmp_string.size(),
                                tmp_string.begin())) {
                            replaceable = true;
                            break;
                        }
                    }


                    if (!replaceable) { //外部言語列に一致部分無し
                        kb_it++;
                        continue; // while
                    } else {
                        break;
                    }
                }
                if (!replaceable) {
                    if (!clipped) {
                        clipped = true;
                        if (!changed) {
                            changed = true;
                        }
                    }
                    clipped_string = tmp_string;
                } else {
                    break;
                }
            } while ((!replaceable) && clipped_string.size() > 1);

            if (clipped)
                *term_strings_it = clipped_string;
        }
    }
    //文ルールの省略語文字列を一つの記号列にする
    term_strings_it = term_strings.begin();
    sym_flag = false;
    term_it = src[0].external.begin();
    for (; term_it != src[0].external.end(); term_it++) {
        switch ((*term_it).type) {
            case ELEM_TYPE::SYM_TYPE:
                if (!sym_flag) {
                    sym_flag = true;
                    buf.insert(buf.end(), (*term_strings_it).begin(), (*term_strings_it).end());
                    term_strings_it++;
                }
                break;
            case ELEM_TYPE::CAT_TYPE:
                sym_flag = false;
                buf.push_back(*term_it);
                break;
        }
    }
    src[0].external = buf;
    res = src;
    return changed;
}

std::vector<Rule>
KnowledgeBase::rules(void) {
    std::vector<Rule> kb_all;
    std::vector<Rule>::iterator sentenceDB_it, wordDB_it;
    sentenceDB_it = sentenceDB.begin();
    for (; sentenceDB_it != sentenceDB.end(); sentenceDB_it++) {
        kb_all.push_back(*sentenceDB_it);
    }
    wordDB_it = wordDB.begin();
    for (; wordDB_it != wordDB.end(); wordDB_it++) {
        kb_all.push_back(*wordDB_it);
    }

    return kb_all;
}

std::vector<std::vector<Element> >
KnowledgeBase::recognize_terminal_strings(Rule& target) {
    std::vector<Element>::iterator it;
    std::vector<std::vector<Element> > terminal_strings;
    std::vector<Element> buf;

    it = target.external.begin();
    for (; it != target.external.end(); it++) {
        switch ((*it).type) {
            case ELEM_TYPE::SYM_TYPE:
                buf.push_back(*it);
                break;
            case ELEM_TYPE::CAT_TYPE:
                if (buf.size() != 0) {
                    std::vector<Element> new_buf;
                    terminal_strings.push_back(new_buf);
                    terminal_strings[terminal_strings.size() - 1] = buf;
                    buf.clear();
                }
                break;
        }
    }
    if (buf.size() != 0) {
        std::vector<Element> new_buf;
        terminal_strings.push_back(new_buf);
        terminal_strings[terminal_strings.size() - 1] = buf;
    }

    return terminal_strings;
}


#ifdef DEBUG_KB

int main(int arg, char **argv) {
    using namespace std;

    Element::load_dictionary((char*) "data.dic");
    Rule buf;
    KnowledgeBase kb;
    std::vector<Rule> vec;

    //chunk1 test
    std::cout << "\n****************chunk1 test" << std::endl;
    vec.push_back(Rule(std::string("S hoge hoge foo -> a b c")));
    vec.push_back(Rule(std::string("S hoge hoge hoge -> a d c")));

    kb.send_box(vec);
    std::cout << kb.to_s() << std::endl;

    kb.chunk();
    std::cout << kb.to_s() << std::endl;

    //chunk2 test
    std::cout << "\n****************chunk2 test" << std::endl;
    vec.clear();
    vec.push_back(Rule(std::string("S hoge hoge foo -> a g c")));
    kb.send_box(vec);
    kb.chunk();
    std::cout << kb.to_s() << std::endl;

    //merge test
    std::cout << "\n****************merge test" << std::endl;
    std::cout << "\n%%% previoud" << std::endl;
    std::cout << kb.to_s() << std::endl;
    buf = Rule(std::string("C:2 hoge -> d"));
    kb.word_box.insert(kb.word_box.begin(), buf);
    kb.merge();
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;

    //replace test
    std::cout << "\n****************replace test" << std::endl;
    std::cout << "\n%%% previoud" << std::endl;
    std::cout << kb.to_s() << std::endl;
    buf = Rule(std::string("C:3 hoge -> c"));
    kb.word_box.insert(kb.word_box.begin(), buf);
    kb.replace();
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;

    //consolidate test
    std::cout << "\n****************consolidate test" << std::endl;
    vec.push_back(Rule(std::string("S hoge hoge foo -> a b c")));
    vec.push_back(Rule(std::string("S hoge hoge hoge -> a d c")));
    vec.push_back(Rule(std::string("C:2 hoge -> d")));
    vec.push_back(Rule(std::string("C:4 hoge -> c")));
    kb.send_box(vec);

    std::cout << "\n%%% previoud" << std::endl;
    std::cout << kb.to_s() << std::endl;
    kb.consolidate();
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;

    //build index test
    std::cout << "\n****************build index test" << std::endl;
    std::map<unsigned int, std::multimap<unsigned int, Rule> >::iterator dit;
    std::multimap<unsigned int, Rule>::iterator item_it;
    kb.build_word_index();

    dit = kb.word_dic.begin();
    while (dit != kb.word_dic.end()) {
        std::cout << "\nNOW... C:" << (*dit).first << std::endl;
        item_it = (*dit).second.begin();
        while (item_it != (*dit).second.end()) {
            std::cout << "ind: " << Element::dic.individual[(*item_it).first] << std::endl;
            std::cout << "rule: " << (*item_it).second.to_s() << std::endl;
            item_it++;
        }
        dit++;
    }

    //fabricate test
    std::cout << "\n****************fabricate test" << std::endl;
    KnowledgeBase kb2;
    Rule input1, input2;
    try {
        //kb2.set_seed(11111111);
        MT19937::irand.engine().seed(11111111);
        input1 = Rule(std::string("S hoge hoge hoge -> d"));
    } catch (const char* msg) {
        std::cout << "ERR:" << msg << std::endl;
        throw;
    }
    std::cout << "prev: " << input1.to_s() << std::endl;

    kb2.fabricate(input1);
    std::cout << "fabr: " << input1.to_s() << std::endl;

    input2 = Rule(std::string("S hoge hoge hoge -> d"));
    input2.external.clear();
    kb.fabricate(input2);
    std::cout << "fabr: " << input2.to_s() << std::endl;
    std::cout << "\n****************end" << std::endl;

    //parse test

    return 1;
}
#endif
