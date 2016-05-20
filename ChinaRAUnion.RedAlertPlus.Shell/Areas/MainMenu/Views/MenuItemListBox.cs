using ChinaRAUnion.RedAlertPlus.Shell.Areas.MainMenu.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace ChinaRAUnion.RedAlertPlus.Shell.Areas.MainMenu.Views
{
    public class MenuItemListBox : ListBox
    {
        protected override void PrepareContainerForItemOverride(DependencyObject element, object item)
        {
            ((ListBoxItem)element).Loaded += MenuItemListBox_Loaded;
            base.PrepareContainerForItemOverride(element, item);
        }

        private void MenuItemListBox_Loaded(object sender, RoutedEventArgs e)
        {
            var item = (ListBoxItem)sender;
            var target = item.DataContext as MenuButtonViewModel;
            target?.SetupMenuItem(item);
        }
    }
}
