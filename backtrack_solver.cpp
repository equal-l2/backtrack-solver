#include <iostream>
#include <iomanip>
#include <array>
#include <bitset>
#include <stack>

#define ENABLE_PROGRESS_PRINT false

class sudoku_cell{
  public:
    using numset = std::bitset<9>;
    sudoku_cell():cells{}{}
    std::array<unsigned,81> cells;

    void solve(){
      std::cout << *this << std::endl;
#if ENABLE_PROGRESS_PRINT
      std::cout << *this;
#endif

      gen_cands();
      back_track();

#if !ENABLE_PROGRESS_PRINT
      std::cout << '\n' << *this;
#else
      std::cout << "\033[11A" << *this;
#endif
      std::cout << std::endl;
    }

  private:
    std::array<numset,81> cands;

    static inline unsigned rc_to_idx(const unsigned row, const unsigned col){
      return 9*row+col;
    }

    static inline unsigned idx_to_row(const unsigned idx){
      return idx/9;
    }

    static inline unsigned idx_to_col(const unsigned idx){
      return idx%9;
    }

    static inline int nth_onbit(const numset& e, unsigned n){ // n is zero-oriented
      int cnt = 0;
      for(int i=0;i<9;++i) if(e[i] && cnt++ == n) return i+1;
      return -1;
    }

    numset cand_in_row(const unsigned idx){
      numset ret;
      ret.set();
      const unsigned row = idx_to_row(idx);
      for(unsigned i=0;i<9;++i) if(cells[rc_to_idx(row,i)] != 0) ret.reset(cells[rc_to_idx(row,i)]-1);
      return ret;
    }

    numset cand_in_col(const unsigned idx){
      numset ret;
      ret.set();
      const unsigned col = idx_to_col(idx);
      for(unsigned i=0;i<9;++i) if(cells[rc_to_idx(i,col)] != 0) ret.reset(cells[rc_to_idx(i,col)]-1);
      return ret;
    }

    numset cand_in_3x3(const unsigned idx){
      numset ret;
      ret.set();
      const unsigned row_begin = idx_to_row(idx)/3*3;
      const unsigned col_begin = idx_to_col(idx)/3*3;
      for(unsigned i=row_begin;i<row_begin+3;++i){
        for(unsigned j=col_begin;j<col_begin+3;++j){
          if(cells[rc_to_idx(i,j)] != 0) ret.reset(cells[rc_to_idx(i,j)]-1);
        }
      }

      return ret;
    }

    void gen_cands(){
      // generate candidates for each cells
      for(unsigned i=0; i<81;++i) if(cells[i] == 0) cands[i] = (cand_in_row(i) & cand_in_col(i) & cand_in_3x3(i));
    }

    bool check_all(){
      for(int i=80;i>=0;--i){
        if(cells[i] == 0 && cands[i].none()) return false;
      }
      return true;
    }

    void back_track(){

      struct trial{
        unsigned idx;
        unsigned tries;
      };

      std::stack<trial> try_stack;

      unsigned begin = 0, end = 81;
      while(begin < 81 && cells[begin] != 0) ++begin;
      while(end > 0 && cells[end] != 0) --end;

      try_stack.push({begin,0});

      unsigned cur = begin;
      while(cur < end){
        if(try_stack.empty()) break;
        trial& t = try_stack.top();
        if(t.idx == cur){
          int num = nth_onbit(cands[cur],t.tries);
          if(num == -1 || !check_all()){
            try_stack.pop();
            unset_cell(cur);
            if(try_stack.empty()) break;
            else{
              trial& t2 = try_stack.top();
              cur = t2.idx;
              unset_cell(t2.idx);
              t2.tries++;
            }
            continue;
          }
          // guaranteed num != -1
          set_cell(cur,num);
        }
        else if(cells[cur] == 0){
          int num = nth_onbit(cands[cur],0);
          if(num == -1 || !check_all()){
            cur = t.idx;
            unset_cell(t.idx);
            t.tries++;
            continue;
          }
          else{
            // guaranteed num != -1
            try_stack.push({cur,0});
            set_cell(cur,num);
          }
        }
        ++cur;
      }
    }

    void set_cell(unsigned idx, unsigned val){
#if ENABLE_PROGRESS_PRINT
      std::cout << "\033[11A" << *this;
#endif

      cells[idx] = val;
      const unsigned row = idx_to_row(idx);
      const unsigned col = idx_to_col(idx);
      const unsigned row_begin = row/3*3;
      const unsigned col_begin = col/3*3;
      cands[idx].reset();
      for(unsigned i=0;i<9;++i){
        cands[rc_to_idx(row,i)].reset(val-1);
        cands[rc_to_idx(i,col)].reset(val-1);
      }
      for(unsigned i=row_begin;i<row_begin+3;++i){
        for(unsigned j=col_begin;j<col_begin+3;++j){
          cands[rc_to_idx(i,j)].reset(val-1);
        }
      }
    }

    void unset_cell(unsigned idx){
#if ENABLE_PROGRESS_PRINT
      std::cout << "\033[11A" << *this;
#endif

      cells[idx] = 0;
      gen_cands();
    }


    friend std::ostream& operator<<(std::ostream& os, sudoku_cell& s){
      for (unsigned i=0;i<9;++i){
        os << ' ';
        for(unsigned j=0;j<9;++j){
          os << s.cells[9*i+j] << ((j+1)%3 == 0 && j != 8 ? " | " : " ");
        }
        std::cout << (((i+1)%3 == 0 && i != 8) ? "\n-------+-------+-------\n" : "\n");
      }
      return os;
    }
};

#include <thread>
#include <chrono>

int main(){
  using namespace std::literals::chrono_literals;
  auto t1 = std::chrono::system_clock::now();
  sudoku_cell s;
  s.cells = {{
#include "problem.txt"
  }};

  s.solve();

  auto t2 = std::chrono::system_clock::now();

  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << std::endl;

  // wait for flushing (only for Windows mintty!)
  std::this_thread::sleep_for(0.1s);
}
