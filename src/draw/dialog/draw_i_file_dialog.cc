/**
 * @author Volodymyr Hromakov (4y5t6r@gmail.com)
 */

#include "draw_i_file_dialog.h"

namespace esc::draw {
// ---
void IFileDialog::Show() { dialog_.Open(); }

// ---
void IFileDialog::Draw(const AppState &app_state) {
  dialog_.Display();

  if (!dialog_.HasSelected()) {
    return;
  }

  OnFileSelected(app_state, dialog_.GetSelected().string());
  dialog_.ClearSelected();
}

// ---
IFileDialog::IFileDialog(const ImGui::FileBrowser &dialog) : dialog_{dialog} {}
}  // namespace esc::draw