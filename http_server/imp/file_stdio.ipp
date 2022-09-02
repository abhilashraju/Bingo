file_stdio::~file_stdio() {
  if (f_)
    fclose(f_);
}

file_stdio::file_stdio(file_stdio &&other)
    : f_(boost::exchange(other.f_, nullptr)) {}

file_stdio &file_stdio::operator=(file_stdio &&other) {
  if (&other == this)
    return *this;
  if (f_)
    fclose(f_);
  f_ = other.f_;
  other.f_ = nullptr;
  return *this;
}

void file_stdio::native_handle(FILE *f) {
  if (f_)
    fclose(f_);
  f_ = f;
}

void file_stdio::close() {
  if (f_) {
    int failed = fclose(f_);
    f_ = nullptr;
    if (failed) {
    }
  }
}

void file_stdio::open(char const *path, file_mode mode) {
  if (f_) {
    fclose(f_);
    f_ = nullptr;
  }
  char const *s;
  switch (mode) {
  default:
  case file_mode::read:
    s = "rb";
    break;

  case file_mode::scan:
    s = "rb";
    break;

  case file_mode::write:
    s = "wb+";
    break;

  case file_mode::write_new: {
    s = "wbx";
    break;
  }

  case file_mode::write_existing:
    s = "rb+";
    break;

  case file_mode::append:
    s = "ab";
    break;

  case file_mode::append_existing: {

    auto const f0 = std::fopen(path, "rb+");
    if (!f0) {
      throw bingo::file_not_found(path);
    }
    std::fclose(f0);
    s = "ab";
    break;
  }
  }

  f_ = std::fopen(path, s);
  if (!f_) {
    throw bingo::file_not_found(path);
  }
}

std::uint64_t file_stdio::size() const {
  if (!f_) {
    throw std::runtime_error("Cannot Open File");
  }
  long pos = std::ftell(f_);
  if (pos == -1L) {
    throw std::runtime_error("Cannot Open File");
  }
  int result = std::fseek(f_, 0, SEEK_END);
  if (result != 0) {
    throw std::runtime_error("Cannot Open File");
  }
  long size = std::ftell(f_);
  if (size == -1L) {
    throw std::runtime_error("Cannot Open File");
  }
  result = std::fseek(f_, pos, SEEK_SET);
  if (result != 0)
    throw std::runtime_error("Cannot Open File");
  return size;
}

std::uint64_t file_stdio::pos() const {
  if (!f_) {
    throw std::runtime_error("Cannot Open File");
  }
  long pos = std::ftell(f_);
  if (pos == -1L) {
    throw std::runtime_error("Cannot Open File");
  }
  return pos;
}

void file_stdio::seek(std::uint64_t offset) {
  if (!f_) {
    throw std::runtime_error("Cannot Open File");
  }
  if (offset > (std::numeric_limits<long>::max)()) {
    throw std::runtime_error("Cannot Open File");
  }
  int result = std::fseek(f_, static_cast<long>(offset), SEEK_SET);
  if (result != 0)
    throw std::runtime_error("Cannot Open File");
}

std::size_t file_stdio::read(void *buffer, std::size_t n) const {
  if (!f_) {
    throw std::runtime_error("Cannot Open File");
  }
  auto nread = std::fread(buffer, 1, n, f_);
  if (std::ferror(f_)) {
    throw std::runtime_error("Cannot Open File");
  }
  return nread;
}

std::size_t file_stdio::write(void const *buffer, std::size_t n) {
  if (!f_) {
    throw std::runtime_error("Cannot Open File");
  }
  auto nwritten = std::fwrite(buffer, 1, n, f_);
  if (std::ferror(f_)) {
    throw std::runtime_error("Cannot Open File");
  }
  return nwritten;
}