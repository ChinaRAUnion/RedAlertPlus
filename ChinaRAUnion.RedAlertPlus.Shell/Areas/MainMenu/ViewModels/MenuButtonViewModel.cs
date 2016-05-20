using ChinaRAUnion.RedAlertPlus.Primitives;
using ChinaRAUnion.RedAlertPlus.Shell.Areas.MainMenu.Views;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomato.Mvvm;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace ChinaRAUnion.RedAlertPlus.Shell.Areas.MainMenu.ViewModels
{
    public class MenuButtonViewModel : BindableBase
    {
        private MenuItem _item;
        public string Text => _item?.Text ?? string.Empty;
        public bool IsEnabled => _item?.IsEnabled ?? false;

        public bool HasContent => _item != null;
        private bool _isAnimationPlaying;
        public bool IsAnimationPlaying
        {
            get { return _isAnimationPlaying; }
            set
            {
                if (SetProperty(ref _isAnimationPlaying, value))
                    OnIsAnimationPlayingChanged();
            }
        }

        private ListBoxItem _menuItem;
        private readonly MenuPresenter _presenter;
        private State _state = State.Stable;

        public MenuButtonViewModel(MenuPresenter presenter, MenuItem item)
        {
            _presenter = presenter;
            _item = item;
        }

        public void Invoke()
        {
            _item?.Invoke();
        }

        public void OnClick()
        {
            _presenter.RequestInvoke(this);
        }

        public void SetupMenuItem(ListBoxItem sender)
        {
            _menuItem = sender;
            _presenter.ReportLoaded();
        }

        public void GoToStableState()
        {
            _state = State.Stable;
            VisualStateManager.GoToState(_menuItem, nameof(State.Stable) +
                (HasContent ? string.Empty : "NoContent"), true);
        }

        public void GoToEnteringState()
        {
            _state = State.Entering;
            VisualStateManager.GoToState(_menuItem, nameof(State.Entering) +
                (HasContent ? string.Empty : "NoContent"), true);
        }

        public void OnIsAnimationPlayingChanged()
        {
            if (!IsAnimationPlaying)
            {
                switch (_state)
                {
                    case State.Stable:
                        break;
                    case State.Entering:
                        GoToStableState();
                        _presenter.ReportEntered();
                        break;
                    case State.Leaving:
                        _presenter.ReportLeaved();
                        _state = State.Leaved;
                        break;
                    default:
                        break;
                }
            }
        }

        public void GoToShowContentState()
        {
            if (HasContent)
                VisualStateManager.GoToState(_menuItem, "ShowContent", true);
        }

        public void GoToNoContentState()
        {
            if (HasContent)
                VisualStateManager.GoToState(_menuItem, "NoContent", true);
        }

        public void GoToLeavingState()
        {
            GoToNoContentState();

            _state = State.Leaving;
            VisualStateManager.GoToState(_menuItem, nameof(State.Leaving) +
                (HasContent ? string.Empty : "NoContent"), true);
        }

        enum State
        {
            Stable,
            Entering,
            Leaving,
            Leaved
        }
    }
}
