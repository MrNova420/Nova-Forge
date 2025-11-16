/**
 * Custom Redux Selector Hook
 * Typed selector hook for use throughout the application
 */

import { useSelector } from 'react-redux';
import type { RootState } from '../store/store';

export const useAppSelector = <TSelected>(
  selector: (state: RootState) => TSelected
): TSelected => {
  return useSelector(selector);
};
