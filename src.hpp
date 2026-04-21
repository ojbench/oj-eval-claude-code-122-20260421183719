#ifndef SRC_HPP
#define SRC_HPP

#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>

// Abstract base class for events
class Event {
public:
  Event() = delete;
  Event(const std::string &name, int deadline) : name_(name), deadline_(deadline), complete_(false) {}

  Event(const Event &) = delete;
  Event(Event &&) = delete;
  Event &operator=(const Event &) = delete;
  Event &operator=(Event &&) = delete;

  virtual ~Event() = default;

  const std::string &GetName() const { return name_; }
  void SetComplete() { complete_ = true; }
  bool IsComplete() const { return complete_; }
  int GetDeadline() const { return deadline_; }

  virtual std::string GetNotification(int n) const = 0;

private:
  std::string name_;
  int deadline_;
  bool complete_;
};

class NormalEvent final : public Event {
public:
  NormalEvent(const std::string &name, int deadline) : Event(name, deadline) {}

  std::string GetNotification(int n) const override {
    if (n != 0) {
      throw std::runtime_error("Notification argument is invalid for Normal Events!");
    }
    return "Normal Event \"" + GetName() + "\" is over.";
  }
};

class NotifyBeforeEvent final : public Event {
public:
  NotifyBeforeEvent(const std::string &name, int deadline, int notify_time)
    : Event(name, deadline), notify_time_(notify_time) {}

  int GetNotifyTime() const { return notify_time_; }

  std::string GetNotification(int n) const override {
    if (n == 0) {
      return "Notify Before Event \"" + GetName() + "\" is about to end. Please hurry!";
    }
    if (n == 1) {
      return "Notify Before Event \"" + GetName() + "\" is over.";
    }
    throw std::runtime_error("Notification argument is invalid for Notify Before Events!");
  }

private:
  int notify_time_;
};

class NotifyLateEvent : public Event {
public:
  NotifyLateEvent(const std::string &name, int deadline, int frequency)
    : Event(name, deadline), frequency_(frequency) {}

  int GetFrequency() const { return frequency_; }

  std::string GetNotification(int n) const override {
    if (n == 0) {
      return "Notify Late Event \"" + GetName() + "\" is over.";
    }
    if (n > 0) {
      return "Notify Late Event \"" + GetName() + "\" is late for " + std::to_string(frequency_ * n) + " hours. ";
    }
    throw std::runtime_error("Notification argument is invalid for Notify Late Events!");
  }

private:
  int frequency_;
};

class CustomNotifyLateEvent final : public NotifyLateEvent {
public:
  CustomNotifyLateEvent(const std::string &name, int deadline, int frequency, std::string (*generator)(int))
    : NotifyLateEvent(name, deadline, frequency), generator_(generator) {}

  std::string GetNotification(int n) const override {
    // 1) Get base notification from NotifyLateEvent
    std::string base = NotifyLateEvent::GetNotification(n);
    // 2) Generate custom part
    std::string custom = generator_ ? generator_(n) : std::string();
    // 3) Concatenate both parts
    return base + custom;
  }

private:
  std::string (*generator_)(int);
};

class Memo {
 public:
  Memo() = delete;

  // duration indicates we simulate hours 1..duration
  explicit Memo(int duration) : duration_(duration), current_time_(0) {}

  ~Memo() = default;

  void AddEvent(const Event *event) {
    events_.push_back(event);
  }

  void Tick() {
    // advance one hour
    ++current_time_;
    if (duration_ > 0 && current_time_ > duration_) {
      return;
    }

    for (const Event *e : events_) {
      if (!e || e->IsComplete()) continue;

      // Deadline time
      const int t = current_time_;
      const int d = e->GetDeadline();

      // Try downcast to specific types to decide schedule
      if (auto nb = dynamic_cast<const NotifyBeforeEvent *>(e)) {
        int notify_time = nb->GetNotifyTime();
        if (t == d - notify_time) {
          std::cout << nb->GetNotification(0) << '\n';
        }
        if (t == d) {
          std::cout << nb->GetNotification(1) << '\n';
        }
        continue;
      }

      if (auto nl = dynamic_cast<const NotifyLateEvent *>(e)) {
        if (t == d) {
          std::cout << nl->GetNotification(0) << '\n';
        } else if (t > d) {
          int freq = nl->GetFrequency();
          if (freq > 0) {
            int delta = t - d;
            if (delta % freq == 0) {
              int n = delta / freq; // n >= 1
              std::cout << nl->GetNotification(n) << '\n';
            }
          }
        }
        continue;
      }

      // NormalEvent (or other Event types): only at deadline
      if (t == d) {
        std::cout << e->GetNotification(0) << '\n';
      }
    }
  }

 private:
  int duration_;
  int current_time_;
  std::vector<const Event *> events_;
};

#endif // SRC_HPP
