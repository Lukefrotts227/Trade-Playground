#include "backtest/portfolio.h"

namespace backtest {

Portfolio::Portfolio(double starting_cash)
    : starting_cash_(starting_cash), cash_(starting_cash), position_(0.0) {}

void Portfolio::apply_signal(Signal signal, const Bar& bar) {
    if (signal == Signal::Buy && position_ == 0.0) {
        position_ = cash_ / bar.close;
        trades_.push_back(Trade{bar.timestamp, TradeSide::Buy, bar.close, position_});
        cash_ = 0.0;
    } else if (signal == Signal::Sell && position_ != 0.0) {
        double quantity = position_;
        cash_ = position_ * bar.close;
        trades_.push_back(Trade{bar.timestamp, TradeSide::Sell, bar.close, quantity});
        position_ = 0.0;
    }
}

double Portfolio::cash() const { return cash_; }

double Portfolio::position() const { return position_; }

double Portfolio::equity(const Bar& bar) const { return cash_ + position_ * bar.close; }

double Portfolio::starting_cash() const { return starting_cash_; }

const std::vector<Trade>& Portfolio::trades() const { return trades_; }

}  // namespace backtest
