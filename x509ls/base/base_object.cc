// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/base/base_object.h"

#include <stddef.h>

#include "x509ls/base/event_manager.h"
#include "x509ls/cli/base/cli_application.h"

namespace x509ls {
BaseObject::BaseObject(BaseObject* parent)
  :
    parent_(parent),
    application_(parent_->GetApplication()) {
  parent_->AddChild(this);
  application_->GetEventManager()->Register(this);
}

BaseObject::BaseObject(CliApplication* application)
  :
    parent_(NULL),
    application_(application) {
  application_->GetEventManager()->Register(this);
}

BaseObject::~BaseObject() {
  application_->GetEventManager()->Unregister(this);

  set<BaseObject*>::iterator it;
  for (it = children_.begin(); it != children_.end(); ++it) {
    DeleteChild(*it);
  }
  children_.clear();
}

void BaseObject::AddChild(BaseObject* child) {
  children_.insert(child);
}

void BaseObject::DeleteChild(BaseObject* child) {
  delete child;
  children_.erase(child);
}

BaseObject* BaseObject::GetParent() const {
  return parent_;
}

CliApplication* BaseObject::GetApplication() const {
  return application_;
}

// virtual
void BaseObject::OnEvent(const BaseObject* source, int event_code) {
}

// virtual
void BaseObject::OnFDEvent(int fd, bool read_event,
    bool write_event, bool error_event) {
}

void BaseObject::Subscribe(BaseObject* source, int event_code) {
  GetApplication()->GetEventManager()->Subscribe(source, this, event_code);
}

void BaseObject::Unsubscribe(BaseObject* source, int event_code) {
  GetApplication()->GetEventManager()->Unsubscribe(source, this, event_code);
}

void BaseObject::Emit(int event_code) {
  GetApplication()->GetEventManager()->EnqueueEvent(this, event_code);
}

void BaseObject::WatchFD(int fd, int fd_events) {
  GetApplication()->GetEventManager()->WatchFD(this, fd, fd_events);
}

void BaseObject::UnwatchFD(int fd) {
  GetApplication()->GetEventManager()->UnwatchFD(this, fd);
}

void BaseObject::EnablePoll() {
  GetApplication()->GetEventManager()->EnablePoll(this);
}

void BaseObject::DisablePoll() {
  GetApplication()->GetEventManager()->DisablePoll(this);
}

// virtual
void BaseObject::OnPoll() {
}

}  // namespace x509ls

